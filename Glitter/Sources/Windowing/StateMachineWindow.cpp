#include "Windowing/StateMachineWindow.hpp"


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>

#include "Helpers/Shared.hpp"

#include "UI/outliner.hpp"

#include "RenderPipeline/ShadowPass.hpp"
#include "RenderPipeline/LightingPass.hpp"
#include "RenderPipeline/PostProcess.hpp"

#include "RenderPipeline/SceneViewport.hpp"

#include <EngineState.hpp>

#include "Camera/Camera.hpp"
#include "Camera/FlyCam.hpp"

#include "Controls/Input.hpp"
#include "Event/EventQueue.hpp"

#include "Lights/Skybox.hpp"
#include "Lights/light.hpp"

#include "3DModel/model.hpp"

#include <filesystem>

#include <cstdio>

StateMachineWindow::StateMachineWindow(GLFWwindow* shareContext)
    : mShareContext(shareContext)
{
}

void StateMachineWindow::init()
{
    initCommonInput();

    // If this window is launched standalone (without EditorWindow/ProjectManagerWindow),
    // GLFW may not be initialized yet.
    // glfwInit() is reference-count-less but is safe to call multiple times; it will
    // simply return GLFW_TRUE after the first successful init.
    if (glfwInit() == GLFW_FALSE)
    {
        const char* desc = nullptr;
        const int err = glfwGetError(&desc);
        std::fprintf(stderr, "[StateMachineWindow] glfwInit failed (%d): %s\n", err, desc ? desc : "(no description)");
        return;
    }

    // Create second GLFW window (shares GL objects with the main window if provided).
    GLFWwindow* share = mShareContext;

    // Ensure this window uses the same GL context settings as the rest of the engine.
    // (GLFW window hints are global, so set them explicitly before creating this window.)
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
    const int w = (mode && mode->width > 0) ? (mode->width / 2) : 1280;
    const int h = (mode && mode->height > 0) ? (mode->height / 2) : 720;

    mWindow = glfwCreateWindow(w, h, "StateMachine", nullptr, share);
    if (!mWindow)
    {
        const char* desc = nullptr;
        const int err = glfwGetError(&desc);
        std::fprintf(stderr,
            "[StateMachineWindow] glfwCreateWindow failed (%d): %s (share=%p, size=%dx%d)\n",
            err, desc ? desc : "(no description)", (void*)share, w, h);
        return;
    }

    if (EngineState::state)
        EngineState::state->mStatemachineWindow = mWindow;

    makeCurrent();
    glfwSwapInterval(1);
    // In the standalone StateMachine app this may be the first window/context created,
    // so we must ensure GL function pointers are loaded before any GL calls
    // (e.g. Shader::Shader -> glCreateShader).
    static bool sGladLoaded = false;
    if (!sGladLoaded)
    {
        if (!gladLoadGL())
        {
            std::fprintf(stderr, "[StateMachineWindow] gladLoadGL failed\n");
            glfwDestroyWindow(mWindow);
            mWindow = nullptr;
            return;
        }
        sGladLoaded = true;
    }

    // Create isolated ImGui contexts for this window
    mImguiContext = Shared::createImguiContext();
    mImNodesContext = Shared::createImNodesContext();

    setImguiCurrent();
    ImNodes::SetCurrentContext(mImNodesContext);
    Shared::initImguiBackendForWindow(mWindow);

    // Attach ImGui contexts for per-window input routing.
    {
        auto* ud = static_cast<WindowInputUserData*>(glfwGetWindowUserPointer(mWindow));
        if (!ud)
        {
            ud = new WindowInputUserData();
            glfwSetWindowUserPointer(mWindow, ud);
        }
        ud->imguiCtx = mImguiContext;
        ud->imnodesCtx = mImNodesContext;
    }

    // Copy of UI setup (like EditorWindow) - will be cleaned up later.
    mNodeGraph = std::make_unique<NodeGraph>();

    // Inline scene setup (mirrors EditorWindow)
    if (EngineState::state)
    {
        mPreviewLevel = std::make_unique<Level>();
        mPreviewLevel->setInputHandler(mInputHandler.get());
        mPreviewLevel->setEventQueue(mQueue.get());
        mPreviewLevel->setEventBus(&EngineState::state->bus);

		// Create a window-local camera for this tool window.
		// GameWindow already allocates mEditorCamera in initCommonInput().
		// Keep it isolated from EditorWindow's EngineState::editorCamera.
		if (mEditorCamera)
    {
      // Place camera so it can see the origin-spawned preview cube.
      mEditorCamera->cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
      mEditorCamera->cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
			mPreviewLevel->cameras.push_back(mEditorCamera.get());
    }

        // NOTE: StateMachine tool window should not load the project's entry level.
        // Instead, create a simple preview scene (cube) so the render pipeline is validated.
        {
            auto engineRoot = EngineState::state->engineInstalledDirectory;
            auto cube = std::make_shared<Model>("./EngineAssets/cube.fbx", engineRoot);
            cube->setTransform(glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(), glm::vec3(1.0f));
            mPreviewLevel->addRenderable(cube);
        }

        auto engineFSPath = std::filesystem::path(EngineState::state->engineInstalledDirectory);
        mSkyBox = std::make_unique<Lighting::Skybox>(engineFSPath, mWindow);

        mLights = std::make_unique<Lights>();
        mLights->initDefaultLights(mPreviewLevel.get());

        // Ensure a default PBR material exists for this window's GL context.
        // (Needed for meshes without an explicit material assigned.)
        EngineState::state->GenerateDefaultMaterials();
    }

    // Per-window 3D viewport renderer. We render the active editor level if present.
    // This keeps StateMachine window lightweight: it doesn't own the world, just a viewport.
    if (EngineState::state)
    {
        mSceneViewport = std::make_unique<SceneViewport>();

        // Prefer using the editor's light system if available; otherwise the viewport will
        // still initialize but render() will be a no-op when required pointers are null.
        // NOTE: EngineState currently doesn't expose Lights* directly; we only initialize
        // ShadowPass if we can find a lights pointer at render time.
        // For now, pass nullptr; SceneViewport::init only uses it to construct ShadowPass.
        // We'll rebuild ShadowPass lazily in tick when lights are available.
        mSceneViewport->init(mWindow, mLights.get());
    }

    // Input for this window (ImGui install_callbacks=false, so we must install callbacks)
    // Use this window's own camera (created by GameWindow).
    Camera* cam = mEditorCamera.get();
    if (cam)
    {
        mInputHandler = std::make_unique<InputHandler>(cam, mWindow, (float)w, (float)h);
        mInputHandler->handleInput(0.0f, *mInputCtx, EngineState::state->isPlay);
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

    // NOTE: GL + ImGui contexts are expected to be made current by the window manager
    // (Editor::openEditor) when this window becomes active.

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- 3D Scene (render directly to this window's backbuffer) ---
    if (mSceneViewport && mPreviewLevel)
    {
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(mWindow, &fbW, &fbH);
        if (fbW > 0 && fbH > 0)
        {
            // Keep render targets in sync with the actual window size.
            mSceneViewport->resize(fbW, fbH);

            // Use this window's own camera.
            Camera* activeCamera = (!mPreviewLevel->cameras.empty())
				? mPreviewLevel->cameras[0]
				: static_cast<Camera*>(mEditorCamera.get());

            if (activeCamera)
            {
                activeCamera->setFrameContext(frameContext(fbW, fbH));
                activeCamera->tick();

                if (mSkyBox)
                    mSkyBox->Draw(activeCamera->viewMatrix(), activeCamera->projectionMatrix());

                mSceneViewport->render(
                    mPreviewLevel->renderables,
                    activeCamera,
                    mLights.get(),
                    mSkyBox ? mSkyBox->getCubeMap() : nullptr,
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


