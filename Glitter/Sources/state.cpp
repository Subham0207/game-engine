#pragma once
#include <EngineState.hpp>

ProjectAsset::UIState &getUIState()
{
    return State::state->uiState;
}

Level &getActiveLevel()
{
   return *State::state->activeLevel;
}

PhysicsSystemWrapper &getPhysicsSystem()
{
    return *State::state->physics;
}
