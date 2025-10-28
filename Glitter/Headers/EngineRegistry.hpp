#pragma once
#include <map>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

namespace Engine
{
    class Registry
    {
        public:
            // main file map {guid: file_system_path}
            std::map<std::string, std::string> renderableSaveFileMap;

            //subsets of file map
            std::map<std::string, std::string> statemachineFileMap;
            //subsets of file map
            std::map<std::string, std::string> animationsFileMap;

            void init();
            void update(std::string guid, std::string filepath);

        private:
            std::map<std::string, std::string> loadMetaFiles(const fs::path& rootDir);
            void updateEachFileTypeMap(std::string guid, std::string filepath);
    };
}