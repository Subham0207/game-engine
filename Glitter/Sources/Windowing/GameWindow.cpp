#include "Windowing/GameWindow.hpp"

#include <EngineState.hpp>

#include "Camera/FlyCam.hpp"

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

    onBeginFrame();
    tickImpl();
}


