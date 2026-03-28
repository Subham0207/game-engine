//
// Created by subha on 22-03-2026.
//

#include "../Headers/EditorStateMachine.hpp"
#include <cstdlib>

#include <EngineState.hpp>
#include <LuaEngine/LuaRegistry.hpp>

#include "Windowing/GameWindow.hpp"

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

EditorStateMachine::EditorStateMachine(std::string characterFilepath, std::string& smFilePath)
{
    // Engine init (once for this process)
    auto* state = new EngineState();
    state->init();
    EngineState::state = state;

    LuaRegistry::SetupLua(
        EngineState::state->luaEngine->state(),
        EngineState::state->currentActiveProjectDirectory);

    // Create and drive the tool window. No share context (separate process).
    window = std::make_unique<StateMachineWindow>(characterFilepath, smFilePath);
    window->init();
}

int EditorStateMachine::openEditor()
{
    while (window && !window->shouldClose())
    {
        glfwPollEvents();
        window->tick();
    }

    if (window)
        window->shutdown();

    glfwTerminate();

    return EXIT_SUCCESS;
}
