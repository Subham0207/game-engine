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

            void init();

        private:
            std::map<std::string, std::string> loadMetaFiles(const fs::path& rootDir);
    };
}