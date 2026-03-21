#include "Windowing/StateMachineWindow.hpp"


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>

#include "Helpers/Shared.hpp"

#include "UI/outliner.hpp"
#include "UI/AssetBrowser/AssetBrowser.hpp"
#include "NodeGraph/NodeGraph.hpp"

#include "RenderPipeline/ShadowPass.hpp"
#include "RenderPipeline/LightingPass.hpp"
#include "RenderPipeline/PostProcess.hpp"

#include <EngineState.hpp>

#include "Camera/Camera.hpp"
#include "Camera/FlyCam.hpp"

#include "Controls/Input.hpp"
#include "Event/EventQueue.hpp"

StateMachineWindow::StateMachineWindow(GLFWwindow* shareContext)
    : mShareContext(shareContext)
{
}

void StateMachineWindow::init()
{
    initCommonInput();

    // Create second GLFW window (shares GL objects with the main window if provided).
    // IMPORTANT: Do NOT call glfwInit() again.
    GLFWwindow* share = mShareContext;

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    const int w = (mode && mode->width > 0) ? (mode->width / 2) : 1280;
    const int h = (mode && mode->height > 0) ? (mode->height / 2) : 720;

    mWindow = glfwCreateWindow(w, h, "StateMachine", nullptr, share);
    if (!mWindow)
        return;

    if (EngineState::state)
        EngineState::state->mStatemachineWindow = mWindow;

    makeCurrent();
    glfwSwapInterval(1);
    // gladLoadGL() only needs to run once per process, but is harmless if loader supports it.
    // We avoid re-calling it here.

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
    mOutliner = std::make_unique<Outliner>();
    mAssetBrowser = std::make_unique<ProjectAsset::AssetBrowser>();
    mNodeGraph = std::make_unique<NodeGraph>();

    // Input for this window (ImGui install_callbacks=false, so we must install callbacks)
    // Reuse editor camera for now; this is just to enable basic input + callbacks.
    FlyCam* editorCam = EngineState::state ? EngineState::state->editorCamera : nullptr;
    Camera* cam = editorCam;
    if (cam)
    {
        mInputHandler = std::make_unique<InputHandler>(cam, mWindow, (float)w, (float)h);
        mInputHandler->handleInput(0.0f, *mInputCtx, EngineState::state->isPlay);
    }
}

void StateMachineWindow::tickImpl()
{
    if (!mWindow) return;

    // NOTE: GL + ImGui contexts are expected to be made current by the window manager
    // (Editor::openEditor) when this window becomes active.

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update callbacks + input routing for this window.
    if (mInputHandler)
        mInputHandler->handleInput(0.0f, *mInputCtx, EngineState::state->isPlay);

    // --- ImGui ---
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("State Machine Editor");
    ImGui::SliderFloat("Split", &mSplitRatio, 0.2f, 0.8f, "%.2f");

    ImVec2 avail = ImGui::GetContentRegionAvail();
    const float leftW = avail.x * mSplitRatio;
    const float rightW = avail.x - leftW;

    ImGui::BeginChild("##SM_Scene", ImVec2(leftW, avail.y), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextUnformatted("Scene");

    unsigned int sceneTex = 0;
    if (EngineState::state && EngineState::state->postProcess)
    {
        // Best-effort: PostProcess likely owns the final screen texture.
        // If no accessor exists, this will remain 0 until you expose one.
        // sceneTex = EngineState::state->postProcess->getScreenTexture();
    }

    if (sceneTex != 0)
    {
        ImVec2 imgAvail = ImGui::GetContentRegionAvail();
        if (imgAvail.x > 0.0f && imgAvail.y > 0.0f)
            ImGui::Image((ImTextureID)(intptr_t)sceneTex, imgAvail, ImVec2(0, 1), ImVec2(1, 0));
    }
    else
    {
        ImGui::TextUnformatted("(No scene texture exposed yet)");
    }

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("##SM_NodeGraph", ImVec2(rightW, avail.y), true);
    ImGui::TextUnformatted("Node Graph");
    // Draw the node graph embedded into this window.
    if (mNodeGraph)
        mNodeGraph->drawUIEmbedded();
    ImGui::EndChild();

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


