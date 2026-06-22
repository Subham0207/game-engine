//
// Created by subha on 04-01-2026.
//

#include "Prefab.hpp"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <algorithm>
#include <cctype>
#include <sstream>

#include "EngineState.hpp"
#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "Helpers/Shared.hpp"
#include "Modals/FileType.hpp"
namespace bs = boost::property_tree;

#include <utility>

namespace
{
    std::vector<std::string> parseTagsCsv(const std::string& csv)
    {
        std::vector<std::string> tags;
        std::stringstream stream(csv);
        std::string token;
        while (std::getline(stream, token, ','))
        {
            token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            token.erase(std::find_if(token.rbegin(), token.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), token.end());
            if (!token.empty())
            {
                tags.push_back(token);
            }
        }
        return tags;
    }

    std::string toTagsCsv(const std::vector<std::string>& tags)
    {
        std::string csv;
        for (size_t i = 0; i < tags.size(); ++i)
        {
            csv += tags[i];
            if (i + 1 < tags.size())
            {
                csv += ",";
            }
        }
        return csv;
    }
}

namespace Engine
{
    void Prefab::writePrefab(fs::path path, PrefabType type)
    {
        switch (type)
        {
            case PrefabType::CharacterPrefab:
            {

                break;
            }
            default:
            break;
        }
    }
    void Prefab::loadFromPrefabFile(fs::path path, PrefabType type)
    {
    }

    void Prefab::readCharacterPrefab(fs::path filepath, CharacterPrefabConfig& character)
    {
        try {
            bs::ptree root;
            // Load the json file into the property tree
            bs::read_json(filepath.string(), root);
            // Accessing top-level values
            character.classId = root.get<std::string>("classId");

            character.modelGuid = root.get<std::string>("model_guid");
            character.modelRelativePosition = glm::vec3(
                root.get<float>("modelRelativePosition.x"),
                root.get<float>("modelRelativePosition.y"),
                root.get<float>("modelRelativePosition.z")
                );
            character.modelScale = glm::vec3(
            root.get<float>("modelScale.x"),
            root.get<float>("modelScale.y"),
            root.get<float>("modelScale.z")
            );


            character.capsuleHalfHeight = root.get<float>("capsuleHalfHeight");
            character.capsuleRadius = root.get<float>("capsuleRadius");
            character.capsulePhysicsLayer = root.get<std::string>("capsule.physicsLayer", "Default");
            character.capsuleIsSensor = root.get<bool>("capsule.isSensor", false);
            character.capsuleMass = root.get<float>("capsule.mass", 80.0f);
            character.capsuleMaxStrength = root.get<float>("capsule.maxStrength", 100.0f);
            character.capsuleFriction = root.get<float>("capsule.friction", 0.2f);
            character.capsuleRestitution = root.get<float>("capsule.restitution", 0.0f);

            character.skeletonGuid = root.get<std::string>("skeleton_guid");
            // Accessing nested values using the dot notation
            character.stateMachineGuid = root.get<std::string>("statemachine_guid");
            character.controllerClassId = root.get<std::string>("playerController.classId");

            // Backward compatible gameplay-tag load path:
            // - preferred: gameplayTagsCsv (new)
            // - fallback: gameplayTags (legacy string csv)
            // - fallback: gameplayTags array (legacy list)
            character.gameplayTags.clear();
            const std::string tagsCsv = root.get<std::string>("gameplayTagsCsv", "");
            if (!tagsCsv.empty())
            {
                character.gameplayTags = parseTagsCsv(tagsCsv);
            }
            else
            {
                const std::string legacyTagsCsv = root.get<std::string>("gameplayTags", "");
                if (!legacyTagsCsv.empty())
                {
                    character.gameplayTags = parseTagsCsv(legacyTagsCsv);
                }
                else if (const auto tagsNode = root.get_child_optional("gameplayTags"))
                {
                    for (const auto& tagNode : *tagsNode)
                    {
                        const std::string tag = tagNode.second.get_value<std::string>("");
                        if (!tag.empty())
                        {
                            character.gameplayTags.push_back(tag);
                        }
                    }
                }
            }
        } catch (const bs::json_parser_error& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        } catch (const bs::ptree_error& e) {
            std::cerr << "Error extracting data: " << e.what() << std::endl;
        }

    }

    void Prefab::writeAIPrefab(fs::path path, std::shared_ptr<AiPrefab> ai)
    {
        try
        {
            bs::ptree root;

            root.put("classId", ai->classId);
            root.put("characterPrefabAssetId", ai->characterPrefabAssetId);

            savePrefab(path, root);
        }
        catch (const bs::json_parser_error& e) {
            std::cerr << "Error writing JSON: " << e.what() << std::endl;
        }
    }

    void Prefab::readAIPrefab(fs::path filepath, std::shared_ptr<AiPrefab> ai)
    {
        try
        {
            bs::ptree root;
            bs::read_json(filepath.string(), root);

            ai->name = filepath.filename().stem().string();
            ai->classId = root.get<std::string>("classId");
            ai->characterPrefabAssetId = root.get<std::string>("characterPrefabAssetId");

        }catch (const bs::json_parser_error& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        } catch (const bs::ptree_error& e) {
            std::cerr << "Error extracting data: " << e.what() << std::endl;
        }
    }

    void Prefab::savePrefab(const fs::path& path, const bs::ptree& root)
    {
        bs::write_json(path.string(), root);

        bs::ptree meta;
        auto guid = boost::uuids::to_string(boost::uuids::random_generator()());
        meta.put("guid", guid);
        meta.put("type", toString(FileType::CharacterType));
        meta.put("version", "0.1");
        meta.put("content.relative_path", path.filename().string());

        const fs::path metaFile = path.parent_path() / (guid +  ".meta.json");
        write_json(metaFile.string(), meta);

        std::cout << "Successfully wrote prefab to " << path.string() << std::endl;
    }

    void Prefab::writeCharacterPrefab(fs::path path, CharacterPrefabConfig& character)
    {
        bs::ptree root;

        // Set top-level values
        root.put("classId", character.classId);

        root.put("model_guid", character.modelGuid);
        root.put("modelRelativePosition.x", character.modelRelativePosition.x);
        root.put("modelRelativePosition.y", character.modelRelativePosition.y);
        root.put("modelRelativePosition.z", character.modelRelativePosition.z);

        root.put("modelScale.x", character.modelScale.x);
        root.put("modelScale.y", character.modelScale.y);
        root.put("modelScale.z", character.modelScale.z);

        root.put("capsuleHalfHeight", character.capsuleHalfHeight);
        root.put("capsuleRadius", character.capsuleRadius);
        root.put("capsule.physicsLayer", character.capsulePhysicsLayer);
        root.put("capsule.isSensor", character.capsuleIsSensor);
        root.put("capsule.mass", character.capsuleMass);
        root.put("capsule.maxStrength", character.capsuleMaxStrength);
        root.put("capsule.friction", character.capsuleFriction);
        root.put("capsule.restitution", character.capsuleRestitution);

        root.put("skeleton_guid", character.skeletonGuid);

        // Create the nested object structure using dot notation
        // Boost will automatically create the "statemachine" node
        root.put("statemachine_guid", character.stateMachineGuid);
        root.put("playerController.classId", character.controllerClassId);
        root.put("gameplayTagsCsv", toTagsCsv(character.gameplayTags));

        try {
            savePrefab(path, root);
        } catch (const bs::json_parser_error& e) {
            std::cerr << "Error writing JSON: " << e.what() << std::endl;
        }
    }
}
