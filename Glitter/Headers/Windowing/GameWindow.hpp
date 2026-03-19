#pragma once

#include <string>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imnodes.h>

// Base class for any standalone GLFW + OpenGL + ImGui window.
//
// Design goals:
// - Each GameWindow owns one GLFWwindow* and its own ImGui/ImNodes contexts.
// - Editor::openEditor() drives all GameWindows from a single loop, switching
//   GL context + ImGui context between windows.
class GameWindow {
public:
    virtual ~GameWindow() = default;

    // Must create window + initialize everything needed to tick.
    virtual void init() = 0;

    // One frame. Implementation should:
    // - make its window/context current
    // - start per-window ImGui frame
    // - draw + render
    // - swap buffers
    virtual void tick() = 0;

    virtual void shutdown() = 0;

    bool shouldClose() const { return mWindow == nullptr || glfwWindowShouldClose(mWindow) != 0; }

    GLFWwindow* window() const { return mWindow; }

protected:
    GLFWwindow* mWindow = nullptr;
    ImGuiContext* mImguiContext = nullptr;
    ImNodesContext* mImNodesContext = nullptr;

    // Convenience helpers for derived classes
    void makeCurrent() const { if (mWindow) glfwMakeContextCurrent(mWindow); }
    void setImguiCurrent() const { if (mImguiContext) ImGui::SetCurrentContext(mImguiContext); }
};

