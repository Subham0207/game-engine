#pragma once

#include <string>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imnodes.h>

#include <memory>

#include "Event/EventQueue.hpp"
#include "Event/InputContext.hpp"

// Needed because GameWindow owns and constructs std::unique_ptr<FlyCam>
#include "Camera/FlyCam.hpp"

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

    // One frame entry point.
    // This is a non-virtual wrapper that:
    // - updates per-window delta time (clamped)
    // - writes EngineState::deltaTime for legacy systems
    // and then calls tickImpl() in the derived class.
    void tick();

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

    // Per-window default camera for input systems. This avoids relying on EngineState
    // (which may be null for standalone executables like ProjectManager).
    std::unique_ptr<FlyCam> mEditorCamera;

    // Per-window timing (avoids using EngineState::lastFrame globally)
    float mLastFrame = 0.0f;
    float mDeltaTime = 0.0f;
    float mMaxDeltaTime = 0.1f;

    // Derived windows implement their frame here.
    virtual void tickImpl() = 0;

    // Called by tick() before tickImpl(). You can override if needed.
    virtual void onBeginFrame() {}

    void initCommonInput()
    {
        mQueue = std::make_unique<EventQueue>();
        mInputCtx = std::make_unique<InputContext>();
        mInputCtx->queue = mQueue.get();

        // Create a default camera for this window.
        // Windows that want to use EngineState's editorCamera can overwrite this pointer.
        if (!mEditorCamera)
            mEditorCamera = std::make_unique<FlyCam>("WindowCamera");
    }

    // Convenience helpers for derived classes
    void makeCurrent() const { if (mWindow) glfwMakeContextCurrent(mWindow); }
    void setImguiCurrent() const { if (mImguiContext) ImGui::SetCurrentContext(mImguiContext); }
};

