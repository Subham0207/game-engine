#pragma once

#include <string>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imnodes.h>

#include <memory>

#include "Event/EventQueue.hpp"
#include "Event/InputContext.hpp"

class InputHandler;

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

    // Common per-window input/event plumbing
    InputHandler* inputHandler() const { return mInputHandler.get(); }
    InputContext* inputContext() const { return mInputCtx.get(); }
    EventQueue* eventQueue() const { return mQueue.get(); }

protected:
    GLFWwindow* mWindow = nullptr;
    ImGuiContext* mImguiContext = nullptr;
    ImNodesContext* mImNodesContext = nullptr;

    // Owned per-window queue + context.
    // InputHandler is optional; derived windows construct it when needed.
    std::unique_ptr<EventQueue> mQueue;
    std::unique_ptr<InputContext> mInputCtx;
    std::unique_ptr<InputHandler> mInputHandler;

    void initCommonInput()
    {
        mQueue = std::make_unique<EventQueue>();
        mInputCtx = std::make_unique<InputContext>();
        mInputCtx->queue = mQueue.get();
    }

    // Convenience helpers for derived classes
    void makeCurrent() const { if (mWindow) glfwMakeContextCurrent(mWindow); }
    void setImguiCurrent() const { if (mImguiContext) ImGui::SetCurrentContext(mImguiContext); }
};

