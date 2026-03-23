#pragma once

#include <string>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imnodes.h>

#include <memory>

#include "Event/EventQueue.hpp"
#include "Event/InputContext.hpp"

#include "Windowing/FrameContext.hpp"

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

    // Screen size tracked per window (window == framebuffer in this engine).
    int screenWidth() const { return mScreenWidth; }
    int screenHeight() const { return mScreenHeight; }
    float aspectRatio() const
    {
        return (mScreenHeight > 0)
            ? (static_cast<float>(mScreenWidth) / static_cast<float>(mScreenHeight))
            : 1.0f;
    }

    FrameContext frameContext() const
    {
        FrameContext ctx;
        ctx.screenWidth = mScreenWidth;
        ctx.screenHeight = mScreenHeight;
        ctx.aspect = aspectRatio();
        return ctx;
    }

    FrameContext frameContext(int targetWidth, int targetHeight) const
    {
        FrameContext ctx;
        ctx.screenWidth = targetWidth;
        ctx.screenHeight = targetHeight;
        ctx.aspect = (targetHeight > 0)
            ? (static_cast<float>(targetWidth) / static_cast<float>(targetHeight))
            : 1.0f;
        return ctx;
    }

    // Common per-window input/event plumbing
    InputHandler* inputHandler() const { return mInputHandler.get(); }
    InputContext* inputContext() const { return mInputCtx.get(); }
    EventQueue* eventQueue() const { return mQueue.get(); }

protected:

    bool mIsVsyncOn = true;
    bool mIsToolWindow = false;
    bool mIsMouseDisabled = false;

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
    std::unique_ptr<FlyCam> mCamera;

    // Per-window timing (avoids using EngineState::lastFrame globally)
    float mLastFrame = 0.0f;
    float mDeltaTime = 0.0f;
    float mMaxDeltaTime = 0.1f;

    // Per-window screen size (pixels). In this engine, window == framebuffer.
    int mScreenWidth = 0;
    int mScreenHeight = 0;

    // Derived windows implement their frame here.
    virtual void tickImpl() = 0;

    // Called by tick() before tickImpl(). You can override if needed.
    virtual void onBeginFrame() {}

    // Refresh cached screen size from GLFW.
    // Call at least once during init (after mWindow is created) and once per frame.
    void updateScreenSize()
    {
        if (!mWindow) return;
        int w = 0, h = 0;
        glfwGetWindowSize(mWindow, &w, &h);
        if (w > 0 && h > 0)
        {
            mScreenWidth = w;
            mScreenHeight = h;
        }
    }

    void initCommonInput(
        bool isToolWindow,
        bool isMouseDisabled,
        const std::string& windowTitle
    );

    // Convenience helpers for derived classes
    void makeCurrent() const { if (mWindow) glfwMakeContextCurrent(mWindow); }
    void setImguiCurrent() const { if (mImguiContext) ImGui::SetCurrentContext(mImguiContext); }
};

