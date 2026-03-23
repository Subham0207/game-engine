#include "Windowing/GameWindow.hpp"

#include <EngineState.hpp>

#include "Camera/FlyCam.hpp"
#include "glad/glad.h"

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

void GameWindow::WindowCreation()
{
    if (glfwInit() == GLFW_FALSE)
    {
        const char* desc = nullptr;
        const int err = glfwGetError(&desc);
        std::fprintf(stderr, "[StateMachineWindow] glfwInit failed (%d): %s\n", err, desc ? desc : "(no description)");
        return;
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
}



