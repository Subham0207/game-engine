//
// Created by subha on 04-01-2026.
//

#include "Prefab.hpp"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "EngineState.hpp"
#include "GenericFactory.hpp"
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

    void Prefab::readCharacterPrefab(fs::path path, CharacterPrefabConfig& character)
    {
        try {
            bs::ptree root;
            // Load the json file into the property tree
            bs::read_json(path.string(), root);
            // Accessing top-level values
            character.classId = root.get<std::string>("classId");
            character.modelGuid = root.get<std::string>("model_guid");
            character.skeletonGuid = root.get<std::string>("skeleton_guid");
            // Accessing nested values using the dot notation
            character.stateMachineClassId = root.get<std::string>("statemachine.classId");
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
        root.put("skeleton_guid", character.skeletonGuid);

        // Create the nested object structure using dot notation
        // Boost will automatically create the "statemachine" node
        root.put("statemachine.classId", character.stateMachineClassId);

        try {
            // Write the property tree to a JSON file
            // The third argument (true) enables pretty-printing (indentation)
            bs::write_json(path.string(), root);
            std::cout << "Successfully wrote prefab to " << path.string() << std::endl;
        } catch (const bs::json_parser_error& e) {
            std::cerr << "Error writing JSON: " << e.what() << std::endl;
        }
    }
}
