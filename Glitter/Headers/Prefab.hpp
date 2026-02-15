//
// Created by subha on 04-01-2026.
//

#ifndef GLITTER_PREFAB_HPP
#define GLITTER_PREFAB_HPP

#include <filesystem>
#include <Character/Character.hpp>

#include "AI/AiPrefab.hpp"
#include "Character/CharacterPrefabConfig.hpp"
#include "Level/Level.hpp"
namespace fs = std::filesystem;

/*
 * Prefab means Pre-fabricated object:
 * It is serialized template that stores the mapping between a class type and its specific assets/data.
 */
namespace Engine
{
    enum PrefabType
    {
        CharacterPrefab
    };

    class Prefab
    {
    public:
        static void writePrefab(fs::path path, PrefabType type);
        static void loadFromPrefabFile(fs::path path, PrefabType type);

        static void writeCharacterPrefab(fs::path path, CharacterPrefabConfig& character);
        static void readCharacterPrefab(fs::path filepath, CharacterPrefabConfig& character);

        static void writeAIPrefab(fs::path path, std::shared_ptr<AiPrefab> ai);
        static void readAIPrefab(fs::path filepath, std::shared_ptr<AiPrefab> ai);
    private:
        static void savePrefab(const fs::path& path, const bs::ptree& root);
    };

}


#endif //GLITTER_PREFAB_HPP