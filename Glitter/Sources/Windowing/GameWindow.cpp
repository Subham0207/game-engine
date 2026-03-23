#include "Windowing/GameWindow.hpp"

#include <EngineState.hpp>

#include "Camera/FlyCam.hpp"
#include "glad/glad.h"
#include "Helpers/Shared.hpp"

void GameWindow::tick()
{
    if (shouldClose())
        return;

    const float currentFrame = static_cast<float>(glfwGetTime());
    float dt = currentFrame - mLastFrame;
    mLastFrame = currentFrame;

    // Clamp to avoid huge physics steps when the window was unfocused / paused.
    if (dt < 0.0f) dt = 0.0f;
    if (dt > mMaxDeltaTime) dt = mMaxDeltaTime;

    mDeltaTime = dt;

    // Keep legacy global for systems still reading EngineState::state->deltaTime.
    if (EngineState::state)
        EngineState::state->deltaTime = mDeltaTime;

    // Update cached screen size (pixels) for this window.
    updateScreenSize();

    onBeginFrame();
    tickImpl();
}

void GameWindow::initCommonInput(
    const bool isToolWindow,
    const bool isMouseDisabled,
    const std::string& windowTitle
    )
{
    mQueue = std::make_unique<EventQueue>();
    mInputCtx = std::make_unique<InputContext>();
    mInputCtx->queue = mQueue.get();

    // Create a default camera for this window.
    // Windows that want to use EngineState's editorCamera can overwrite this pointer.
    if (!mCamera)
        mCamera = std::make_unique<FlyCam>("WindowCamera");

    mIsToolWindow = isToolWindow;
    mIsMouseDisabled = isMouseDisabled;
    mWindow = Shared::initAWindow(
        mIsVsyncOn,
        mIsToolWindow,
        windowTitle,
        mIsMouseDisabled
        );

    Shared::initGpuLogger();
}

