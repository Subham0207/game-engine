#include <EngineState.hpp>
#include <cstdlib>
#include <utility>

EngineState::EngineState(){
    ais = std::vector<AI::AI*>();
    mWindow = nullptr;

    auto value = std::getenv("GLITTER_ENGINE");
    engineInstalledDirectory
     = value != nullptr ? value : fs::current_path().string();

    currentActiveProjectDirectory = "";
}

void EngineState::setEngineDirectory(std::string value)
{
    engineInstalledDirectory = std::move(value);
}

void EngineState::setCurrentDirAsProjectDirectory()
{
    currentActiveProjectDirectory = fs::current_path().string();
}

EngineState* EngineState::state = nullptr;