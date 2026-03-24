#include "Windowing/StateMachineWindow.hpp"


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>

#include "Helpers/Shared.hpp"

#include "UI/outliner.hpp"
#include "RenderPipeline/SceneViewport.hpp"

#include <EngineState.hpp>

#include "Camera/Camera.hpp"
#include "Camera/FlyCam.hpp"

#include "Controls/Input.hpp"
#include "Event/EventQueue.hpp"

#include "Event/EventBus.hpp"

#include "Lights/Skybox.hpp"
#include "Lights/light.hpp"

#include "3DModel/model.hpp"

#include <filesystem>

#include <cstdio>

StateMachineWindow::StateMachineWindow(std::string characterFilePath)
    : mCharacterFilePath(characterFilePath)
{
}

void StateMachineWindow::init()
{
    initWindowAndBackends(true, false, "StateMachine Editor", true);

    if (EngineState::state)
        EngineState::state->mStatemachineWindow = mWindow;

    // Copy of UI setup (like EditorWindow) - will be cleaned up later.
    mNodeGraph = std::make_unique<NodeGraph>();

    // Inline scene setup (mirrors EditorWindow)
    if (EngineState::state)
    {
        mPreviewLevel = std::make_unique<Level>();
        mPreviewLevel->setEventQueue(mQueue.get());
        mPreviewLevel->setEventBus(mBus.get());

		if (mCamera)
        {
            Camera* cam = mCamera.get();
            mInputHandler = std::make_unique<InputHandler>(cam, mWindow, mScreenWidth, mScreenHeight);
            mPreviewLevel->setInputHandler(mInputHandler.get());

            mCamera->cameraPos = glm::vec3(0.0f, 10.0f, 0.0f);
            mCamera->cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
            mPreviewLevel->cameras.push_back(mCamera.get());
        }

        setupLevelObjs();

        auto engineFSPath = std::filesystem::path(EngineState::state->engineInstalledDirectory);
        mSkyBox = std::make_unique<Lighting::Skybox>(engineFSPath, mWindow);

        mLights = std::make_unique<Lights>();
        mLights->initDefaultLights(mPreviewLevel.get());

        EngineState::state->GenerateDefaultMaterials();

        mSceneViewport = std::make_unique<SceneViewport>();
        mSceneViewport->init(mWindow, mLights.get());
    }
}

void StateMachineWindow::tickImpl()
{
    if (!mWindow) return;

    // Ensure GL context is current for this window before any rendering.
    makeCurrent();
    setImguiCurrent();
    if (mImNodesContext)
        ImNodes::SetCurrentContext(mImNodesContext);

    // --- 3D Scene (render directly to this window's backbuffer) ---
    if (mSceneViewport && mPreviewLevel)
    {
        if (mScreenWidth > 0 && mScreenHeight > 0)
        {
            // Keep render targets in sync with the actual window size.
            mSceneViewport->resize(mScreenWidth, mScreenHeight);

            // Use this window's own camera.
            Camera* activeCamera = (!mPreviewLevel->cameras.empty())
				? mPreviewLevel->cameras[0]
				: static_cast<Camera*>(mCamera.get());

            if (activeCamera)
            {
                activeCamera->setFrameContext(frameContext(mScreenWidth, mScreenHeight));
                activeCamera->tick();
                mSceneViewport->render(
                    mPreviewLevel->renderables,
                    activeCamera,
                    mLights.get(),
                    mSkyBox.get(),
                    mDeltaTime);
            }
        }
    }

    // Update callbacks + input routing for this window.
    if (mInputHandler)
        mInputHandler->handleInput(mDeltaTime, *mInputCtx, EngineState::state->isPlay);

    // --- ImGui (UI overlay) ---
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("State Machine Editor");

    ImGui::TextUnformatted("Node Graph");
    // Draw the node graph embedded into this window.
    if (mNodeGraph)
        mNodeGraph->drawUIEmbedded();

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(mWindow);
}

void StateMachineWindow::shutdown()
{
    if (!mWindow) return;

    makeCurrent();
    // Make sure both ImGui and ImNodes are pointing at this window's contexts
    // before shutting down backends / destroying contexts.
    if (mImguiContext)
        ImGui::SetCurrentContext(mImguiContext);
    if (mImNodesContext)
        ImNodes::SetCurrentContext(mImNodesContext);

    Shared::shutdownImguiBackendForWindow();
    if (mImNodesContext)
    {
        ImNodes::DestroyContext(mImNodesContext);
        mImNodesContext = nullptr;
    }
    if (mImguiContext)
    {
        ImGui::DestroyContext(mImguiContext);
        mImguiContext = nullptr;
    }

    glfwDestroyWindow(mWindow);
    mWindow = nullptr;
}

void StateMachineWindow::setupLevelObjs()
{
    auto engineRoot = fs::path(EngineState::state->engineInstalledDirectory);
    auto cube = std::make_shared<Model>((engineRoot / "EngineAssets" / "cube.fbx").string(), engineRoot.string());
    // Large "ground" cube.
    cube->setTransform(glm::vec3(0.0f, -0.5f, 0.0f), glm::quat(), glm::vec3(100.0f, 1.0f, 100.0f));
    mPreviewLevel->addRenderable(cube);

    mPreviewLevel->spawnCharacter(mCharacterFilePath);
}


