#include "Windowing/EditorWindow.hpp"

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

#include <cstring>

EditorWindow::~EditorWindow() = default;

void EditorWindow::init()
{
    initCommonInput(false, true, "Editor", true);

    EngineState::state->mEditorWindow = mWindow;

    // Level / scene setup
    mLevel = std::make_unique<Level>();
    EngineState::state->activeLevel = mLevel.get();
    auto lvl = EngineState::state->activeLevel;
    auto cameraRef = mCamera.get();
    cameraRef->cameraPos = glm::vec3(0.0f, 10.0f, 0.0f);
    EngineState::state->editorCamera = cameraRef;
    lvl->cameras.push_back(cameraRef);

    auto camera = lvl->cameras[EngineState::state->activeCameraIndex];
    mClientHandler = std::make_unique<ClientHandler>();
    mInputHandler = std::make_unique<InputHandler>(camera, mWindow, mScreenWidth, mScreenHeight);
    mInputHandler->movementSpeed = mEditorCameraMoveSpeed;
    mClientHandler->inputHandler = mInputHandler.get();

    // Provide window-local input/event systems to the Level so any spawned Character
    // can receive the correct InputHandler/EventQueue via Level::spawnCharacter().
    lvl->setInputHandler(mInputHandler.get());
    lvl->setEventQueue(mQueue.get());
    lvl->setEventBus(&EngineState::state->bus);

    lvl->loadMainLevelOfCurrentProject();

    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    mSkyBox = std::make_unique<Lighting::Skybox>(engineFSPath, mWindow);

    EngineState::state->GenerateDefaultMaterials();

    mRayCastObjectSelector = std::make_unique<Debug::Raycast>(engineFSPath);

    mLights = std::make_unique<Lights>();
    mLights->initDefaultLights(mLevel.get());

    mOutliner = std::make_unique<Outliner>();
    // Allow Outliner to request opening tool windows (handled by Editor.cpp window manager).
    mOutliner->windowRequests.openStateMachineWindow = &mWindowRequests.openStateMachineWindow;
    mAssetBrowser = std::make_unique<ProjectAsset::AssetBrowser>();
    mNodeGraph = std::make_unique<NodeGraph>();

    mShadowPass = std::make_unique<ShadowPass>(mWindow, mLights.get());
    mLightingPass = std::make_unique<LightingPass>();
    mPostProcess = std::make_unique<PostProcess>();
    EngineState::state->postProcess = mPostProcess.get();
}

void EditorWindow::tickImpl()
{
    if (!mWindow) return;

    // NOTE: GL + ImGui contexts are expected to be made current by the window manager
    // (Editor::openEditor) when this window becomes active.
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    FrameMark;
    ZoneScopedN("EditorWindow Frame");

    mQueue->drain([&](const Event& e)
    {
        EngineState::state->bus.dispatch(e);
    });

    auto activeCamera = mInputHandler ? mInputHandler->m_Camera : nullptr;
    const float deltaTime = mDeltaTime;

    auto& activeLevel = getActiveLevel();
    auto& lvlrenderables = activeLevel.renderables;

    if (mInputHandler)
        mInputHandler->handleInput(deltaTime, *mInputCtx, EngineState::state->isPlay);

    if (EngineState::state->isPlay)
    {
        ZoneScopedN("EditorIsPlay");
        if (!EngineState::state->playerControllers.empty())
            if (auto character = EngineState::state->playerControllers[EngineState::state->activePlayerControllerId]->getCharacter())
            {
                activeCamera = character->camera;
                if (mInputHandler)
                    mInputHandler->m_Camera = activeCamera;
            }

        if (getPhysicsSystem().isFirstPhysicsEnabledFrame == true)
        {
            getPhysicsSystem().isFirstPhysicsEnabledFrame = false;
            for (int i = 0; i < lvlrenderables.size(); i++)
                lvlrenderables.at(i)->syncTransformationToPhysicsEntity();
        }
        else
        {
            getPhysicsSystem().Update(deltaTime);
            for (int i = 0; i < lvlrenderables.size(); i++)
                lvlrenderables.at(i)->physicsUpdate();
        }
    }
    else
    {
        if (mInputHandler)
            mInputHandler->m_Camera = activeLevel.cameras[0];
        getPhysicsSystem().isFirstPhysicsEnabledFrame = true;
    }

    if (!activeCamera)
        return;

    mPostProcess->attachFBO();

    // Keep per-window render targets in sync with the actual window size.
    // (In this engine, window == framebuffer.)
    if (mPostProcess)
        mPostProcess->resize(screenWidth(), screenHeight());

    // Provide per-frame window sizing info to the camera without changing Camera::tick().
    activeCamera->setFrameContext(frameContext());
    activeCamera->tick();

    mSkyBox->Draw(activeCamera->viewMatrix(), activeCamera->projectionMatrix());

    for (auto& i : mLights->pointLights)
    {
        ZoneScopedN("EditorPointLightSelection");
        i.position = i.lightModel->GetPosition();
        if (i.lightModel->getIsSelected())
            getUIState().propretiesPanel->pointLight = &i;
    }

    for (auto& i : mLights->directionalLights)
    {
        ZoneScopedN("EditorPointDirectionalLightSelection");
        if (i.lightModel->getIsSelected())
            getUIState().propretiesPanel->directionalLight = &i;
    }

    for (auto& i : mLights->spotLights)
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

    getActiveLevel().tickAIs(deltaTime);

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

    mPostProcess->draw(
        *mShadowPass,
        *mLightingPass,
        lvlrenderables,
        activeCamera,
        mLights.get(),
        mSkyBox->getCubeMap(),
        deltaTime);

    for (int i = 0; i < getActiveLevel().textSprites.size(); i++)
        getActiveLevel().textSprites.at(i)->RenderText3D(activeCamera->viewMatrix(), activeCamera->projectionMatrix());

    // --- ImGui ---
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (EngineState::state->isDevMode)
    {
        // Keep handler in sync with UI-edited speed.
        mInputHandler->movementSpeed = mEditorCameraMoveSpeed;

        mRayCastObjectSelector->HandleSelection(
            frameContext(),
            mOutliner.get(),
            activeCamera,
            &activeLevel,
            mInputHandler.get());

        mOutliner->Render(*mLevel, mEditorCameraMoveSpeed);
        mAssetBrowser->RenderAssetBrowser();
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

    if (!mFirstFrame)
    {
        if (!EngineState::state->isDevMode) EngineState::state->isPlay = true;
        mFirstFrame = true;
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






