//
// Created by subha on 13-03-2026.
//

#ifndef GLITTER_PROJECTMANIFEST_HPP
#define GLITTER_PROJECTMANIFEST_HPP
#pragma once
#include <filesystem>
#include <optional>
#include <map>
namespace fs = std::filesystem;

struct Development
{
    std::string engineDir;
    std::string projectDir;
};

class ProjectManifest
{
    public:
        explicit ProjectManifest(fs::path path);
        ProjectManifest(std::string name, fs::path projectManagerDir, std::string levelGuidFileName);

        bool isDevelopment() const { return development.has_value();}

        std::string getEngineDir(){ return development->engineDir; }
        std::string getProjectDir(){ return development->projectDir; }

        void save(const fs::path& destinationPath);

        [[nodiscard]] std::string getEntryLevel() const
        {
            return entryLevel;
        }

    private:
        std::string version;
        std::string name;
        std::string id;
        std::optional<Development> development;
        std::map<std::string, fs::path> mounts;
        std::string entryLevel;
        std::string resolveEngineDir(std::string engineDir);


    // Using static constexpr is memory-efficient and evaluated at compile time
    static constexpr const char* VERSION        = "version";
    static constexpr const char* NAME           = "name";
    static constexpr const char* ID             = "id";
    static constexpr const char* ENTRY_LEVEL = "entryLevel";

    static constexpr const char* MOUNTS         = "mounts";

    static constexpr const char* ASSETS_KEY = "assets";
    static constexpr const char* ASSETS_DIR = "/Assets";
    static constexpr const char* LEVELS_KEY = "levels";
    static constexpr const char* LEVELS_DIR = "/Levels";

    static constexpr const char* DEVELOPMENT    = "development";

    static constexpr const char* ENGINE_DIR   = "engineDir";
    static constexpr const char* PROJECT_DIR  = "projectDir";
};


#endif //GLITTER_PROJECTMANIFEST_HPP