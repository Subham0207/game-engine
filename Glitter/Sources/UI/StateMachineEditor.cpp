//
// Created by subha on 18-03-2026.
//

// NOTE: This file is currently a WIP placeholder.
//
// The project vendored ImGui version does not appear to include multi-viewport
// support, and creating a second GLFW window with the same ImGui backend causes
// OpenGL/ImGui state conflicts.
//
// Recommended approach for now: keep state machine editing inside the main
// ImGui context (single window), or upgrade ImGui and then enable multi-viewport.

#include "UI/StateMachineEditor.hpp"

#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>

#include <EngineState.hpp>
#include <Helpers/Shared.hpp>
#include <Profiler.hpp>

#include "NodeGraph/NodeGraph.hpp"
#include "Lights/light.hpp"
#include "Lights/cubemap.hpp"

namespace UI
{
    StateMachineEditor::StateMachineEditor(std::filesystem::path engineRoot)
        : mEngineRoot(std::move(engineRoot))
    {
    }

    StateMachineEditor::~StateMachineEditor()
    {
        close();
    }

    bool StateMachineEditor::open()
    {
        if (mWindow)
            return true;

        // Create a second GLFW window that shares the OpenGL context with the main window.
        GLFWwindow* share = EngineState::state ? EngineState::state->mEditorWindow : nullptr;
        if (!share)
            return false;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        mWindow = glfwCreateWindow(1280, 720, "State Machine Editor", nullptr, share);
        if (!mWindow)
            return false;

        glfwMakeContextCurrent(mWindow);
        gladLoadGL();

        // This project initializes the ImGui GLFW backend for the main window.
        // The upstream imgui_impl_glfw backend is not designed to be initialized twice.
        // For now we *do not* initialize another ImGui GLFW backend instance here.
        //
        // Rendering in this window will work (OpenGL context is shared), but ImGui input
        // will continue to be driven by the main window until the backend is refactored
        // to support multiple windows (or switched to ImGui multi-viewport).

        if (!mNodeGraph)
            mNodeGraph = new NodeGraph();

        return true;
    }

    void StateMachineEditor::close()
    {
        if (mNodeGraph)
        {
            delete mNodeGraph;
            mNodeGraph = nullptr;
        }

        if (mWindow)
        {
            glfwDestroyWindow(mWindow);
            mWindow = nullptr;
        }
    }

    void StateMachineEditor::beginFrame()
    {
        glfwMakeContextCurrent(mWindow);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void StateMachineEditor::endFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(mWindow);
    }

    void StateMachineEditor::drawLeftSceneViewport(
        const std::vector<std::shared_ptr<Renderable>>& renderables,
        Camera* activeCamera,
        Lights* lights,
        CubeMap* cubeMap,
        float deltaTime,
        int viewportX,
        int viewportY,
        int viewportW,
        int viewportH)
    {
        // Render directly into the left half of the window via glViewport.
        // This is intentionally a minimal approach (no offscreen texture yet).
        glViewport(viewportX, viewportY, viewportW, viewportH);

        // Ensure depth test for scene rendering.
        glEnable(GL_DEPTH_TEST);

        // If you want a "basic PBR scene" independent of the main level,
        // you can later swap renderables/camera here.
        if (lights && cubeMap)
        {
            // ShadowPass expects its own Lights* in ctor; current design keeps it external.
            // Using a locally owned shadow pass is tricky without constructing Lights.
            // For now, we skip shadows in this window and just run the lighting pass.
            mLightingPass.draw(renderables, activeCamera, lights, cubeMap, deltaTime);
        }

        // Restore full viewport for UI.
        int w, h;
        glfwGetFramebufferSize(mWindow, &w, &h);
        glViewport(0, 0, w, h);
    }

    void StateMachineEditor::drawRightNodeGraph()
    {
        if (mNodeGraph)
            mNodeGraph->drawUI();
    }

    void StateMachineEditor::drawDockspaceAndSplitUI()
    {
        // Fullscreen host window.
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        // ImGui::SetNextWindowViewport() is only available in newer ImGui versions.

        ImGuiWindowFlags host_flags = 0;
        host_flags |= ImGuiWindowFlags_NoTitleBar;
        host_flags |= ImGuiWindowFlags_NoCollapse;
        host_flags |= ImGuiWindowFlags_NoResize;
        host_flags |= ImGuiWindowFlags_NoMove;
        host_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        host_flags |= ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("StateMachineEditorHost", nullptr, host_flags);

        ImGui::TextUnformatted("State Machine Editor");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200);
        ImGui::SliderFloat("Split", &mSplitRatio, 0.2f, 0.8f, "%.2f");

        ImGui::Separator();

        // Create two child regions.
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float leftW = avail.x * mSplitRatio;
        float rightW = avail.x - leftW;

        ImGui::BeginChild("LeftViewport", ImVec2(leftW, avail.y), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::TextUnformatted("PBR View");
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("RightNodeGraph", ImVec2(rightW, avail.y), true);
        drawRightNodeGraph();
        ImGui::EndChild();

        ImGui::End();
    }

    bool StateMachineEditor::tick(
        const std::vector<std::shared_ptr<Renderable>>& renderables,
        Camera* activeCamera,
        Lights* lights,
        CubeMap* cubeMap,
        float deltaTime)
    {
        if (!mWindow)
            return false;

        ZoneScopedN("StateMachineEditor::tick");

        if (glfwWindowShouldClose(mWindow))
            return false;

        beginFrame();

        // Determine left panel's framebuffer rect.
        int fbW, fbH;
        glfwGetFramebufferSize(mWindow, &fbW, &fbH);
        int leftW = static_cast<int>(static_cast<float>(fbW) * mSplitRatio);

        // Render scene into left half BEFORE drawing ImGui.
        // NOTE: this draws behind ImGui; later we can render to texture and show it in ImGui.
        drawLeftSceneViewport(renderables, activeCamera, lights, cubeMap, deltaTime, 0, 0, leftW, fbH);

        drawDockspaceAndSplitUI();

        endFrame();

        return true;
    }
}




