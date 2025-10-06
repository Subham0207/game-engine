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

LuaEngine& getLuaEngine()
{
    return *State::state->luaEngine;
}

std::map<std::string, std::string> getEngineRegistryFilesMap()
{
    return State::state->engineRegistry->renderableSaveFileMap;
}