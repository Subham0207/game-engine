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
#include <Physics/box.hpp>
#include <EngineState.hpp>
namespace fs = std::filesystem;

const int MAX_PROJECTS = 5;

static void write_text(const fs::path& p, const std::string& text)
{
    fs::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::binary);
    f << text;
}

int openAProject(const std::string& currentDir)
{
    State::state->currentActiveProjectDirectory = currentDir;
    
    // start the editor.

    //load the main lvl from this project.
    return 0;
}

void update_recent_projects_list(const fs::path& projects_file_path, const fs::path& new_project_path) {
    fs::path absolute_new_path = fs::absolute(new_project_path);

    // 2. Add the new project path to the top of the list.
    // First, remove any existing entry for this project to avoid duplicates.
    getUIState().recent_projects.erase(
        std::remove_if(getUIState().recent_projects.begin(), getUIState().recent_projects.end(),
            [&](const fs::path& p) { return fs::absolute(p) == absolute_new_path; }),
        getUIState().recent_projects.end());

    // Insert the new project at the beginning of the vector.
    getUIState().recent_projects.insert(getUIState().recent_projects.begin(), new_project_path);

    // 3. Trim the list to the maximum size.
    if (getUIState().recent_projects.size() > MAX_PROJECTS) {
        getUIState().recent_projects.resize(MAX_PROJECTS);
    }

    // 4. Write the updated list back to the file.
    // The `std::ofstream` constructor with `std::ios::trunc` will overwrite the file.
    std::ofstream outfile(projects_file_path, std::ios::trunc);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << projects_file_path << std::endl;
        return;
    }
    for (const auto& path : getUIState().recent_projects) {
        outfile << path.string() << std::endl;
    }
    outfile.close();

    std::cout << "Successfully updated recent projects file: " << projects_file_path << std::endl;
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

    State::state->currentActiveProjectDirectory = root.string();

    // Manifest (keep it tiny; add fields as you grow)
    auto lvl = new Level();

    auto floorBox = new Model("./EngineAssets/cube.fbx");
    floorBox->attachPhysicsObject(new Physics::Box(&getPhysicsSystem(), false, true));
    floorBox->save(root/ "Assets");
    lvl->addRenderable(floorBox);
    
    auto character = new Character("./EngineAssets/Aj.fbx");
    character->capsuleColliderPosRelative = glm::vec3(0.0f,-2.5f,0.0f);
    character->save(root/ "Assets");
    lvl->addRenderable(character);

    lvl->save(root / "Levels");

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
        "defaultLevel": "Levels/)" + lvl->GetGuid() + R"(",
        "plugins": []
        }
        )";

    write_text(root / "Project.manifest.json", manifest);

    update_recent_projects_list(fs::path(State::state->engineInstalledDirctory) / "user_prefs.json", root);


    std::cout << "Created project '" << projectName << "' at " << root << "\n";
    std::cout << "Project ID: " << projectId << "\n";
    return 0;
}