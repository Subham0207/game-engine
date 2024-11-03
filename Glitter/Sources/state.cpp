#pragma once
#include <EngineState.hpp>

ProjectAsset::UIState &getUIState()
{
    return State::state->uiState;
}