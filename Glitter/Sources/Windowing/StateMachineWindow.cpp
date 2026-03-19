#include "Windowing/StateMachineWindow.hpp"

#include "Helpers/glitter.hpp"

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

namespace
{
    struct StateMachineWindowState
    {
        Outliner* outliner = nullptr;
        ProjectAsset::AssetBrowser* assetBrowser = nullptr;
        NodeGraph* nodeGraph = nullptr;

        // Temporary: show a scene texture if available from EngineState::state->postProcess.
        float splitRatio = 0.5f;
    };

    StateMachineWindowState g;
}

void StateMachineWindow::init()
{
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

    makeCurrent();
    // gladLoadGL() only needs to run once per process, but is harmless if loader supports it.
    // We avoid re-calling it here.

    // Create isolated ImGui contexts for this window
    mImguiContext = Shared::createImguiContext();
    mImNodesContext = Shared::createImNodesContext();

    setImguiCurrent();
    ImNodes::SetCurrentContext(mImNodesContext);
    Shared::initImguiBackendForWindow(mWindow);

    // Copy of UI setup (like EditorWindow) - will be cleaned up later.
    g.outliner = new Outliner();
    g.assetBrowser = new ProjectAsset::AssetBrowser();
    g.nodeGraph = new NodeGraph();
}

void StateMachineWindow::tick()
{
    if (!mWindow) return;

    makeCurrent();
    setImguiCurrent();
    if (mImNodesContext) ImNodes::SetCurrentContext(mImNodesContext);

    // --- ImGui ---
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("State Machine Editor");
    ImGui::SliderFloat("Split", &g.splitRatio, 0.2f, 0.8f, "%.2f");

    ImVec2 avail = ImGui::GetContentRegionAvail();
    const float leftW = avail.x * g.splitRatio;
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
    g.nodeGraph->drawUIEmbedded();
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


