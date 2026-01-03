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

void EngineState::setCurrentActiveProjectDir(std::string value)
{
    currentActiveProjectDirectory = std::move(value);
}

fs::path EngineState::navIntoProjectDir(std::string path)
{
    return fs::path(state->engineInstalledDirectory) / path;
}

fs::path EngineState::navIntoEnginDir(std::string path)
{
    return fs::path(state->currentActiveProjectDirectory) / path;
}

EngineState* EngineState::state = nullptr;