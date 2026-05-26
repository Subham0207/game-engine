#include "Windowing/EditorWindow.hpp"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>
#include "Controls/Input.hpp"
#include "Controls/ClientHandler.hpp"
#include "Controls/PlayerController.hpp"
#include <EngineState.hpp>
#include "Helpers/Shared.hpp"
#include "Level/Level.hpp"
#include "Lights/Skybox.hpp"
#include "Lights/light.hpp"
#include "Debug/Raycast.hpp"
#include "UI/outliner.hpp"
#include "UI/AssetBrowser/AssetBrowser.hpp"
#include "NodeGraph/NodeGraph.hpp"
#include "RenderPipeline/PostProcess.hpp"
#include "Camera/FlyCam.hpp"
#include <UI/PropertiesPanel.hpp>
#include "UI/Hud/HudSystem.hpp"
#include <Profiler.hpp>

EditorWindow::~EditorWindow() = default;

void EditorWindow::init()
{
    initWindowAndBackends(false, true, "Editor", true);

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
    lvl->setEventBus(mBus.get());

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

    mSceneViewport = std::make_unique<SceneViewport>();
    mSceneViewport->init(mWindow, mLights.get());
    EngineState::state->postProcess = mSceneViewport->getPostProcess();

    mHudSystem = std::make_unique<UI::Hud::HudSystem>();

    const fs::path engineHudDir = engineFSPath / "EngineAssets" / "UI";
    const fs::path engineHudRml = engineHudDir / "hud.rml";
    const fs::path engineHudRcss = engineHudDir / "hud.rcss";

    const fs::path projectHudDir = fs::path(EngineState::state->currentActiveProjectDirectory) / "Assets" / "HUD";
    const fs::path projectHudRml = projectHudDir / "hud.rml";
    const fs::path projectHudRcss = projectHudDir / "hud.rcss";

    if ((fs::exists(engineHudRml) && !fs::exists(projectHudRml)) || (fs::exists(engineHudRcss) && !fs::exists(projectHudRcss)))
    {
        std::error_code ec;
        fs::create_directories(projectHudDir, ec);
        if (fs::exists(engineHudRml) && !fs::exists(projectHudRml))
            fs::copy_file(engineHudRml, projectHudRml, fs::copy_options::overwrite_existing, ec);
        if (fs::exists(engineHudRcss) && !fs::exists(projectHudRcss))
            fs::copy_file(engineHudRcss, projectHudRcss, fs::copy_options::overwrite_existing, ec);
    }

    const fs::path hudDocumentPath = fs::exists(projectHudRml) ? projectHudRml : engineHudRml;
    std::cout << "[HUD] Project HUD rml: " << projectHudRml << " exists=" << fs::exists(projectHudRml) << std::endl;
    std::cout << "[HUD] Project HUD rcss: " << projectHudRcss << " exists=" << fs::exists(projectHudRcss) << std::endl;
    std::cout << "[HUD] Engine HUD rml: " << engineHudRml << " exists=" << fs::exists(engineHudRml) << std::endl;
    std::cout << "[HUD] Resolved HUD document: " << hudDocumentPath << std::endl;
    mHudSystem->init(mWindow, mScreenWidth, mScreenHeight, hudDocumentPath);
}

void EditorWindow::tickImpl()
{
    if (!mWindow) return;

    FrameMark;
    ZoneScopedN("EditorWindow Frame");

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
            //Change Camera when a Character is Possessed.
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
            //When play is stopped make the EditorCamera active.
            mInputHandler->m_Camera = activeLevel.cameras[0];
        getPhysicsSystem().isFirstPhysicsEnabledFrame = true;
    }

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

    if (mSceneViewport && mLevel)
    {
        if (mScreenWidth > 0 && mScreenHeight > 0)
        {
            // Keep render targets in sync with the actual window size.
            mSceneViewport->resize(mScreenWidth, mScreenHeight);

            //Camera selecting logic based on Play has already occurred before so we have an activeCamera.
            if (activeCamera)
            {
                activeCamera->setFrameContext(frameContext(mScreenWidth, mScreenHeight));
                activeCamera->tick();
                mSceneViewport->render(
                    mLevel->renderables,
                    activeCamera,
                    mLights.get(),
                    mSkyBox.get(),
                    mDeltaTime);
            }
        }
    }

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
            [ptr = mOutliner.get()]() { return ptr->GetSelectedIndex(); },
            [ptr = mOutliner.get()](int val) { ptr->setSelectedIndex(val); },
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

    // Draw HUD last so it stays visible above scene/editor windows.
    if (mHudSystem && EngineState::state->isPlay)
        mHudSystem->tick(mScreenWidth, mScreenHeight);

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

    if (mHudSystem)
    {
        mHudSystem->shutdown();
        mHudSystem.reset();
    }

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






