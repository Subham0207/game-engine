#include <EngineState.hpp>

EngineState::EngineState(){
    ais = std::vector<AI::AI*>();
    mWindow = nullptr;
}

EngineState* EngineState::state = nullptr;