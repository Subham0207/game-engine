#pragma once
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <fstream>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <Level/Level.hpp>
#include <EngineState.hpp>
namespace fs = std::filesystem;

static void write_text(const fs::path& p, const std::string& text)
{
    fs::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::binary);
    f << text;
}

int create_new_project(const std::string& currentDir, const std::string& projectName)
{
    fs::path root = fs::path(currentDir) / projectName;
    if (fs::exists(root)) {
        std::cerr << "Target path exists: " << root << "\n";
        return 1;
    }

    auto projectId = boost::uuids::to_string(boost::uuids::random_generator()()); 

    // Folders
    fs::create_directories(root / "Assets");
    fs::create_directories(root / "Levels");
    fs::create_directories(root / "Config");
    fs::create_directories(root / "Library"); // cache/imports (ignore in VCS)

    // Manifest (keep it tiny; add fields as you grow)
    std::string manifest = std::string(R"({
        "engineVersion": "0.1.0",
        "projectName": ")") + projectName + R"(",
        "projectId": ")" + projectId + R"(",
        "mounts": {
            "assets": "Assets/",
            "scenes": "Levels/",
            "config": "Config/",
            "cache": "Library/"
        },
        "defaultScene": "Levels/Main.lvl",
        "plugins": []
        }
        )";

    write_text(root / "Project.manifest.json", manifest);

    auto lvl = new Level();
    lvl->saveToFile((root / "Levels" / "Main.lvl").string(), *lvl);

    State::state->currentActiveProjectDirectory = root.string();

    std::cout << "Created project '" << projectName << "' at " << root << "\n";
    std::cout << "Project ID: " << projectId << "\n";
    return 0;
}