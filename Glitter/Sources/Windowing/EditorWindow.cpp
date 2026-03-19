#include "Windowing/EditorWindow.hpp"

#include "Helpers/glitter.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdlib>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>

#include "Controls/Input.hpp"
#include "Controls/ClientHandler.hpp"
#include "Controls/PlayerController.hpp"
#include "Event/EventQueue.hpp"
#include "Event/InputContext.hpp"

#include <EngineState.hpp>
#include "Helpers/Shared.hpp"

#include "Level/Level.hpp"
#include "Lights/Skybox.hpp"
#include "Lights/light.hpp"

#include "Debug/Raycast.hpp"

#include "UI/outliner.hpp"
#include "UI/AssetBrowser/AssetBrowser.hpp"
#include "NodeGraph/NodeGraph.hpp"

#include "RenderPipeline/ShadowPass.hpp"
#include "RenderPipeline/LightingPass.hpp"
#include "RenderPipeline/PostProcess.hpp"

#include "Camera/FlyCam.hpp"
#include <UI/PropertiesPanel.hpp>

#include <Profiler.hpp>

namespace
{
    struct EditorWindowState
    {
        EventQueue queue;
        InputContext inputCtx;

        Level* level = nullptr;
        Lighting::Skybox* skyBox = nullptr;
        Debug::Raycast* rayCastObjectSelector = nullptr;
        Lights* lights = nullptr;

        Outliner* outliner = nullptr;
        ProjectAsset::AssetBrowser* assetBrowser = nullptr;
        NodeGraph* nodeGraph = nullptr;

        ShadowPass* shadowPass = nullptr;
        LightingPass* lightingPass = nullptr;
        PostProcess* postProcess = nullptr;

        bool firstFrame = false;
    };

    // One global for now; mirrors old openEditor local vars.
    // Later you can move this into the class if desired.
    EditorWindowState g;
}

void EditorWindow::init()
{
    g.inputCtx.queue = &g.queue;

    // Window + GL/ImGui backends
    // NOTE: This function previously also initialized engine registry + physics.
    // We keep that behavior for the main editor window.
    EngineState::state->mWindow = Shared::InitBackEndsWithWindow();
    mWindow = EngineState::state->mWindow;

    // IMPORTANT (Windows / multi-window): we want isolated backend state per GLFWwindow.
    // Shared::InitBackEndsWithWindow() currently calls Shared::initImguiBackend() which
    // creates a global context + initializes imgui_impl_glfw/opengl3 for this window.
    // We immediately shutdown those backends and recreate a dedicated context for this window.
    // This prevents state leakage between multiple windows and avoids WndProc recursion.
    Shared::shutdownImguiBackendForWindow();
    ImNodes::DestroyContext();
    ImGui::DestroyContext();

    mImguiContext = Shared::createImguiContext();
    mImNodesContext = Shared::createImNodesContext();
    setImguiCurrent();
    ImNodes::SetCurrentContext(mImNodesContext);
    Shared::initImguiBackendForWindow(mWindow);

    // Level / scene setup
    g.level = new Level();
    EngineState::state->activeLevel = g.level;
    auto lvl = EngineState::state->activeLevel;
    lvl->cameras.push_back(EngineState::state->editorCamera);

    auto camera = lvl->cameras[EngineState::state->activeCameraIndex];
    ClientHandler::clientHandler = new ClientHandler();
    ClientHandler::clientHandler->inputHandler = new InputHandler(camera, mWindow, 800, 600);
    InputHandler::currentInputHandler = ClientHandler::clientHandler->inputHandler;

    lvl->loadMainLevelOfCurrentProject();

    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    g.skyBox = new Lighting::Skybox(engineFSPath, mWindow);

    EngineState::state->GenerateDefaultMaterials();

    g.rayCastObjectSelector = new Debug::Raycast(engineFSPath);

    g.lights = new Lights();
    g.lights->initDefaultLights();

    g.outliner = new Outliner();
    g.assetBrowser = new ProjectAsset::AssetBrowser();
    g.nodeGraph = new NodeGraph();

    Controls::PlayerController::register_bindings(getLuaEngine());

    Shared::initGpuLogger();

    g.shadowPass = new ShadowPass(mWindow, g.lights);
    g.lightingPass = new LightingPass{};
    g.postProcess = new PostProcess{};
    EngineState::state->postProcess = g.postProcess;
}

void EditorWindow::tick()
{
    if (!mWindow) return;

    makeCurrent();
            glEnable(GL_DEPTH_TEST);
            glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (mImguiContext)
        ImGui::SetCurrentContext(mImguiContext);
    if (mImNodesContext)
        ImNodes::SetCurrentContext(mImNodesContext);

    FrameMark;
    ZoneScopedN("EditorWindow Frame");

    g.queue.drain([&](const Event& e)
    {
        EngineState::state->bus.dispatch(e);
    });

    auto activeCamera = InputHandler::currentInputHandler->m_Camera;
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - EngineState::state->lastFrame;
    EngineState::state->deltaTime = deltaTime;
    EngineState::state->lastFrame = currentFrame;

    auto& activeLevel = getActiveLevel();
    auto& lvlrenderables = activeLevel.renderables;

    ClientHandler::clientHandler->inputHandler->handleInput(EngineState::state->deltaTime, g.inputCtx);

    if (EngineState::state->isPlay)
    {
        ZoneScopedN("EditorIsPlay");
        if (!EngineState::state->playerControllers.empty())
            if (auto character = EngineState::state->playerControllers[EngineState::state->activePlayerControllerId]->getCharacter())
            {
                activeCamera = character->camera;
                ClientHandler::clientHandler->inputHandler->m_Camera = activeCamera;
            }

        if (getPhysicsSystem().isFirstPhysicsEnabledFrame == true)
        {
            getPhysicsSystem().isFirstPhysicsEnabledFrame = false;
            for (int i = 0; i < lvlrenderables.size(); i++)
                lvlrenderables.at(i)->syncTransformationToPhysicsEntity();
        }
        else
        {
            getPhysicsSystem().Update(EngineState::state->deltaTime);
            for (int i = 0; i < lvlrenderables.size(); i++)
                lvlrenderables.at(i)->physicsUpdate();
        }
    }
    else
    {
        InputHandler::currentInputHandler->m_Camera = activeLevel.cameras[0];
        getPhysicsSystem().isFirstPhysicsEnabledFrame = true;
    }

    g.postProcess->attachFBO();

    activeCamera->tick();

    g.skyBox->Draw(activeCamera->viewMatrix(), activeCamera->projectionMatrix());

    for (auto& i : g.lights->pointLights)
    {
        ZoneScopedN("EditorPointLightSelection");
        i.position = i.lightModel->GetPosition();
        if (i.lightModel->getIsSelected())
            getUIState().propretiesPanel->pointLight = &i;
    }

    for (auto& i : g.lights->directionalLights)
    {
        ZoneScopedN("EditorPointDirectionalLightSelection");
        if (i.lightModel->getIsSelected())
            getUIState().propretiesPanel->directionalLight = &i;
    }

    for (auto& i : g.lights->spotLights)
    {
        ZoneScopedN("EditorPointSpotLightSelection");
        i.position = i.lightModel->GetPosition();
        if (i.lightModel->getIsSelected())
            getUIState().propretiesPanel->spotlight = &i;
    }

    if (!getActiveLevel().isNavMeshSetup)
    {
        ZoneScopedN("EditorOneTimeLevelNavMeshSetup");
        getActiveLevel().BuildLevelNavMesh();
        getActiveLevel().isNavMeshSetup = true;
    }
    else
    {
        if (getUIState().renderNavMesh)
            getActiveLevel().renderDebugNavMesh(activeCamera);
    }

    getActiveLevel().tickAIs(EngineState::state->deltaTime);

    for (int i = 0; i < lvlrenderables.size(); i++)
    {
        ZoneScopedN("EditorUpdateBoneMatrixOnCPU");
        if (lvlrenderables.at(i)->ShouldRender())
        {
            if (auto character = std::dynamic_pointer_cast<Character>(lvlrenderables.at(i)))
            {
                if (character->animator)
                    character->updateFinalBoneMatrix(deltaTime);
            }
        }
    }

    g.postProcess->draw(
        *g.shadowPass,
        *g.lightingPass,
        lvlrenderables,
        activeCamera,
        g.lights,
        g.skyBox->getCubeMap(),
        deltaTime);

    for (int i = 0; i < getActiveLevel().textSprites.size(); i++)
        getActiveLevel().textSprites.at(i)->RenderText3D(activeCamera->viewMatrix(), activeCamera->projectionMatrix());

    // --- ImGui ---
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (EngineState::state->isDevMode)
    {
        g.rayCastObjectSelector->HandleSelection(
            g.outliner,
            activeCamera,
            &activeLevel);

        g.outliner->Render(*g.level);
        g.assetBrowser->RenderAssetBrowser();
    }

    {
        ZoneScopedN("ImGuiRender");
        ImGui::Render();
    }
    {
        ZoneScopedN("ImGuiRenderDrawData");
        {
            TracyGpuZone("ImGui Draw");
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }

    glfwSwapBuffers(mWindow);

    TracyGpuCollect;

    if (!g.firstFrame)
    {
        if (!EngineState::state->isDevMode) EngineState::state->isPlay = true;
        g.firstFrame = true;
    }
}

void EditorWindow::shutdown()
{
    if (!mWindow) return;

    makeCurrent();

    // Shutdown this window's isolated backends + contexts.
    setImguiCurrent();
    if (mImNodesContext)
        ImNodes::SetCurrentContext(mImNodesContext);

    Shared::shutdownImguiBackendForWindow();
    if (mImNodesContext) { ImNodes::DestroyContext(mImNodesContext); mImNodesContext = nullptr; }
    if (mImguiContext) { ImGui::DestroyContext(mImguiContext); mImguiContext = nullptr; }

    glfwDestroyWindow(mWindow);
    mWindow = nullptr;
}






