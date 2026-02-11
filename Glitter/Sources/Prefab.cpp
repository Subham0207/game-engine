//
// Created by subha on 04-01-2026.
//

#include "Prefab.hpp"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "EngineState.hpp"
#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "Helpers/Shared.hpp"
#include "Modals/FileType.hpp"
namespace bs = boost::property_tree;

#include <utility>

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

            character.capsuleHalfHeight = root.get<float>("capsuleHalfHeight");
            character.capsuleHalfHeight = root.get<float>("capsuleRadius");

            character.skeletonGuid = root.get<std::string>("skeleton_guid");
            // Accessing nested values using the dot notation
            character.stateMachineClassId = root.get<std::string>("statemachine.classId");
            character.playerControllerClassId = root.get<std::string>("playerController.classId");
        } catch (const bs::json_parser_error& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        } catch (const bs::ptree_error& e) {
            std::cerr << "Error extracting data: " << e.what() << std::endl;
        }

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
        root.put("capsuleHalfHeight", character.capsuleHalfHeight);
        root.put("capsuleRadius", character.capsuleRadius);

        root.put("skeleton_guid", character.skeletonGuid);

        // Create the nested object structure using dot notation
        // Boost will automatically create the "statemachine" node
        root.put("statemachine.classId", character.stateMachineClassId);
        root.put("playerController.classId", character.playerControllerClassId);

        try {
            // Write the property tree to a JSON file
            // The third argument (true) enables pretty-printing (indentation)
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
        } catch (const bs::json_parser_error& e) {
            std::cerr << "Error writing JSON: " << e.what() << std::endl;
        }
    }
}
