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
            // {guid: file_system_path}
            std::map<std::string, std::string> renderableSaveFileMap;

            void init();

        private:
            std::map<std::string, std::string> loadMetaFiles(const fs::path& rootDir);
    };
}