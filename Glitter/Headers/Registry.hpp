#pragma once
#include <map>
#include <string>

namespace Engine
{
    class Registry
    {
        public:
            // {guid: file_system_path}
            std::map<std::string, std::string> renderableSaveFileMap;
    };
}