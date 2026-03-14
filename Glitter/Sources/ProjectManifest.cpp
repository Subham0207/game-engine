//
// Created by subha on 13-03-2026.
//

#include "../Headers/ProjectManifest.hpp"
#include "boost/property_tree/ptree.hpp"
#include <boost/json.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>

#include "Helpers/Shared.hpp"
namespace bs = boost::property_tree;
namespace json = boost::json;

ProjectManifest::ProjectManifest(fs::path path)
{
    // 1. Load the file into a string buffer
    std::ifstream ifs(path);
    std::string str((std::istreambuf_iterator<char>(ifs)),
                     std::istreambuf_iterator<char>());

    // 2. Parse into a boost::json::value
    json::value jv = json::parse(str);
    json::object const& obj = jv.as_object();

    // 3. Extract basic strings
    // .at() throws if key is missing; .value_to converts types safely
    version       = json::value_to<std::string>(obj.at(VERSION));
    name          = json::value_to<std::string>(obj.at(NAME));
    id            = json::value_to<std::string>(obj.at(ID));
    entryLevel = json::value_to<std::string>(obj.at(ENTRY_LEVEL));

    // 4. Extract Mounts (Iterating through an object)
    if (auto* mounts_ptr = obj.if_contains(MOUNTS)) {
        for (auto const& field : mounts_ptr->as_object()) {
            mounts[std::string(field.key())] = json::value_to<std::string>(field.value());
        }
    }

    // 5. Extract Development Block (The Optional Part)
    if (auto* dev_ptr = obj.if_contains(DEVELOPMENT)) {
        auto const& dev_obj = dev_ptr->as_object();

        auto engineDir = json::value_to<std::string>(dev_obj.at(ENGINE_DIR));
        engineDir = resolveEngineDir(engineDir);

        //After project is build manifest file is copied into build directory which is two level deep.
        //So to get to actual project directory we need to travel two levels up
        auto parentPath = path.parent_path();
        auto projectDir = json::value_to<std::string>(dev_obj.at(PROJECT_DIR));
        projectDir = fs::weakly_canonical(parentPath / projectDir).string();


        development = Development{
            engineDir,
            projectDir
        };
    }
}

ProjectManifest::ProjectManifest(std::string name, fs::path projectManagerDir, std::string levelGuidFileName)
        : name(name)
{
    version = "0.1.0";
    id = Shared::uuid(); // Or some unique identifier logic

    // Setup Dev Info immediately
    Development dev;
    dev.projectDir = "../../";
    dev.engineDir = fs::absolute(projectManagerDir).string();
    development = dev;

    // Setup default mounts
    mounts[ASSETS_KEY] = ASSETS_DIR;
    mounts[LEVELS_KEY] = LEVELS_DIR;
    entryLevel = std::move(levelGuidFileName);
}

std::string ProjectManifest::resolveEngineDir(std::string engineDir)
{
    std::string resolvedEngineDir;
    if (engineDir.empty())
    {
        resolvedEngineDir = std::getenv("GLITTER_ENGINE");
    }
    resolvedEngineDir = engineDir;

    if (!exists(fs::path(resolvedEngineDir)))
    {
        std::cout << "Engine Folder " << resolvedEngineDir << " is invalid" << std::endl;
    }

    return resolvedEngineDir;
}

void ProjectManifest::save(const fs::path& destinationPath) {
    json::object obj;

    // 1. Basic Metadata
    obj[VERSION] = version;
    obj[NAME]    = name;
    obj[ID]      = id;
    obj[ENTRY_LEVEL]   = entryLevel;

    // 2. Determine the "Anchor" for relative paths
    // We want to save everything relative to the directory where the manifest lives.
    fs::path manifestDir = destinationPath.parent_path();

    // 3. Serialize Mounts
    json::object mounts_obj;
    for (auto const& [key, relPath] : mounts) {
        mounts_obj[key] = fs::path(relPath).generic_string();
    }
    obj[MOUNTS] = mounts_obj;

    // 4. Serialize Development Block (Optional)
    if (development) {
        json::object dev_obj;

        // We save these as strings.
        dev_obj[ENGINE_DIR]  = development->engineDir;
        dev_obj[PROJECT_DIR] = development->projectDir;

        obj[DEVELOPMENT] = dev_obj;
    }

    // 5. Write to File
    std::ofstream ofs(destinationPath);
    if (ofs.is_open()) {
        // json::serialize produces the string.
        // Use json::parse_options if you want pretty-printing.
        ofs << json::serialize(obj);
    } else {
        throw std::runtime_error("Failed to open manifest for writing: " + destinationPath.string());
    }
}