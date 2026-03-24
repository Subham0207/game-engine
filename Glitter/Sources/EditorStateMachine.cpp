//
// Created by subha on 22-03-2026.
//

#include "../Headers/EditorStateMachine.hpp"
#include <cstdlib>

#include <EngineState.hpp>
#include <LuaEngine/LuaRegistry.hpp>

#include "Windowing/StateMachineWindow.hpp"
#include "Windowing/GameWindow.hpp"

#include <GLFW/glfw3.h>

int EditorStateMachine::openEditor(std::string characterFilepath)
{
    // Engine init (once for this process)
    auto* state = new EngineState();
    state->init();
    EngineState::state = state;

    LuaRegistry::SetupLua(
        EngineState::state->luaEngine->state(),
        EngineState::state->currentActiveProjectDirectory);

    // Create and drive the tool window. No share context (separate process).
    auto window = std::make_unique<StateMachineWindow>(characterFilepath);
    window->init();

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
