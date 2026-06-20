#include "EditorAnimation.hpp"

#include <cstdlib>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

#include <EngineState.hpp>
#include <LuaEngine/LuaRegistry.hpp>

EditorAnimation::EditorAnimation()
{
    auto* state = new EngineState();
    state->init();
    EngineState::state = state;

    LuaRegistry::SetupLua(
        EngineState::state->luaEngine->state(),
        EngineState::state->currentActiveProjectDirectory);

    mWindow = std::make_unique<AnimationEditorWindow>();
    mWindow->init();
}

int EditorAnimation::openEditor()
{
    while (mWindow && !mWindow->shouldClose())
    {
        glfwPollEvents();
        mWindow->tick();
    }

    if (mWindow)
        mWindow->shutdown();

    glfwTerminate();
    return EXIT_SUCCESS;
}
