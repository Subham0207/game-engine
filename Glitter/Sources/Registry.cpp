#include <iostream>
#include <EngineRegistry.hpp>
#include <EngineState.hpp>
#include <Helpers/Shared.hpp>
#include <Modals/FileType.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
namespace bs = boost::property_tree;

void Engine::Registry::init()
{
    //scan the current active project directory for .meta files, create a fs::path files[] of allowed types.
    auto currentProjectDirectory = fs::path(EngineState::state->currentActiveProjectDirectory);
    renderableSaveFileMap = loadMetaFiles(currentProjectDirectory);

    //load subsets
    for (auto &[key, value] : renderableSaveFileMap)
    {
        updateEachFileTypeMap(key, value);
    }
    
}

void Engine::Registry::update(std::string guid, std::string filepath)
{
    renderableSaveFileMap[guid] = filepath;
    updateEachFileTypeMap(guid, filepath);
}

std::map<std::string, std::string> Engine::Registry::loadMetaFiles(const fs::path& rootDir) {
    std::map<std::string, std::string> guidToFileMap;

    for (const auto& entry : fs::recursive_directory_iterator(rootDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            if (entry.path().filename().string().find(".meta") != std::string::npos) {
                try {
                    bs::ptree tree;
                    bs::read_json(entry.path().string(), tree);

                    // Safely get the properties, throws if missing
                    auto guid = tree.get<std::string>("guid");
                    auto fileName = tree.get<std::string>("content.relative_path");

                    guidToFileMap[guid] = (entry.path().parent_path() / fileName).string();
                }
                catch (const bs::ptree_error& e) {
                    std::cerr << "Error reading/parsing " << entry.path()
                              << ": " << e.what() << "\n";
                }
            }
        }
    }

    return guidToFileMap;
}

void Engine::Registry::updateEachFileTypeMap(std::string guid, std::string filepath)
{
    auto suffix = ".statemachine";
    if(Shared::endsWith(filepath, suffix))
    {
        statemachineFileMap[guid] = filepath;
    }

    suffix = ".animation";
    if(Shared::endsWith(filepath, suffix))
    {
        animationsFileMap[guid] = filepath;
    }
}
