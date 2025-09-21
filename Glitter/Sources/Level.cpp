#include  <Level/Level.hpp>
#include <EngineState.hpp>

void Level::loadMainLevelOfCurrentProject()
{
    // read the project.manifest file and find the which level is entry point
    auto manifestDir = fs::path(State::state->currentActiveProjectDirectory) / "project.manifest.json";
    bs::ptree manifest;
    bs::read_json(manifestDir.string(), manifest);
    auto defaultLevelFilePath = fs::path(manifest.get<std::string>("defaultLevel"));
    auto parentPath = defaultLevelFilePath.parent_path();
    auto filename = defaultLevelFilePath.filename().stem().string(); // filename without extension
    //load the level
    this->load(parentPath, filename);
}