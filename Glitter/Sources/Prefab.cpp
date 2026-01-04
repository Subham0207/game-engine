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
        switch (type)
        {
            case PrefabType::CharacterPrefab:
            {
                readCharacterPrefab(std::move(path));
                break;
            }
            default:
                break;
        }
    }

    void Prefab::readCharacterPrefab(fs::path path)
    {
        try {
            bs::ptree root;

            // Load the json file into the property tree
            bs::read_json(path.string(), root);

            // Accessing top-level values
            auto characterClassId = root.get<std::string>("classId");
            auto modelGuid = root.get<std::string>("model_guid");
            auto skeleton_guid = root.get<std::string>("skeleton_guid");

            // Accessing nested values using the dot notation
            auto stateMachineClassId = root.get<std::string>("statemachine.classId");

            // Outputting results
            std::cout << "Class ID: " << characterClassId << "\n"
                      << "Model GUID: " << modelGuid << "\n"
                      << "SM Class ID: " << stateMachineClassId << std::endl;

            //Find the classes and create objects and assign character obj to level renderables
            auto character = CharacterFactory::Create(characterClassId);
            auto filesMap = getEngineRegistryFilesMap();
            if (const auto it = filesMap.find(modelGuid); it != filesMap.end())
            {
                auto model = new Model();
                auto modelParentPath = fs::path(filesMap[modelGuid]).parent_path();
                model->load(modelParentPath, modelGuid);
                character->model = model;
                character->model_guid = modelGuid;
            }
            if (const auto it = filesMap.find(skeleton_guid); it != filesMap.end())
            {
                auto skeleton = new Skeleton::Skeleton();
                auto skeletonParentPath = fs::path(filesMap[modelGuid]).parent_path();
                skeleton->load(skeletonParentPath, modelGuid);
                character->skeleton = skeleton;
                character->skeleton_guid = skeleton_guid;
            }
            if (!stateMachineClassId.empty())
            {
                auto statemachine = StateMachineFactory::Create(stateMachineClassId);
            }

            getActiveLevel().renderables.emplace_back(character);

        } catch (const bs::json_parser_error& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        } catch (const bs::ptree_error& e) {
            std::cerr << "Error extracting data: " << e.what() << std::endl;
        }

    }

    void Prefab::writeCharacterPrefab(fs::path path, std::shared_ptr<::Character> character)
    {
        bs::ptree root;

        // Set top-level values
        root.put("classId", character->GetClassId());
        root.put("model_guid", character->model_guid);
        root.put("skeleton_guid", character->skeleton_guid);

        // Create the nested object structure using dot notation
        // Boost will automatically create the "statemachine" node
        root.put("statemachine.classId", character->animStateMachine->GetClassId());

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
