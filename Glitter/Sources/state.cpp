#pragma once
#include <EngineState.hpp>

ProjectAsset::UIState &getUIState()
{
    return EngineState::state->uiState;
}

Level &getActiveLevel()
{
   return *EngineState::state->activeLevel;
}

PhysicsSystemWrapper &getPhysicsSystem()
{
    return *EngineState::state->physics;
}

LuaEngine& getLuaEngine()
{
    return *EngineState::state->luaEngine;
}

std::map<std::string, std::string> getEngineRegistryFilesMap()
{
    return EngineState::state->engineRegistry->renderableSaveFileMap;
}