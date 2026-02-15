//
// Created by subha on 31-12-2025.
//

#ifndef GLITTER_GENERICFACTORY_HPP
#define GLITTER_GENERICFACTORY_HPP
#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>

class Character;
namespace Controls
{
    class StateMachine;
    class PlayerController;
    class Controller;
}
template <typename TBase>
class GenericFactory {
public:
    using CreatorFunc = std::function<std::shared_ptr<TBase>()>;

    static void Register(const std::string& id, CreatorFunc func) {
        GetTable()[id] = func;
    }

    static std::shared_ptr<TBase> Create(const std::string& id) {
        auto it = GetTable().find(id);
        if (it != GetTable().end()) {
            return it->second();
        }
        return nullptr;
    }
    static std::map<std::string, CreatorFunc>& GetTable() {
        static std::map<std::string, CreatorFunc> table;
        return table;
    }
private:
};

//This is the factory responsible to create any derived class's objects, that are defined in a project.
//Make sure you register the derived class so the engine knows about it.
using CharacterFactory = GenericFactory<Character>;
using StateMachineFactory = GenericFactory<Controls::StateMachine>;
using ControllerFactory = GenericFactory<Controls::Controller>;

#define REGISTER_BODY(ClassName) \
public: \
std::string GetClassId() const override { return #ClassName; }

// General Registration macro
// ClassName: The derived class
// Key: The string ID used for lookup
// FactoryType: The alias (e.g., CharacterFactory or StateMachineFactory)
#define REGISTER_TYPE(ClassName, Key, FactoryType) \
static struct ClassName##Registrar { \
ClassName##Registrar() { \
FactoryType::Register(Key, []() { \
return std::make_shared<ClassName>(); \
}); \
} \
} global_##ClassName##Registrar;

#endif //GLITTER_GENERICFACTORY_HPP

/* DOC
*  we can import character directly in editor UI.
             what this really does is -- Adds serialized character ref to .lvl file. (This serialized version is created to/from character base class in memory by default)
             To support scripting, We need to derive a class from character base class -- so we can override functionalities.
             then when we save. Add any more serializable types and serialize the derived class.
             .lvl file should know which class was used to serialize the character so it can use that class to load the object in memory.

            Create MyCharacter .h .cpp files.
            Can we load MyCharacter at runtime somehow. We can need to compile the project with engine somehow figure how this would link.
            Then when we import a character model this derived class needs to be used to create character object

                Generic Factory Register
                {
                 "ClassId": [] () {return std::unqiueptr<ClassId>()},
                 "MyCharacter": []() { return std::unqiueptr<MyCharacter>()}
                }

                How to create MyCharacter. Don't create this yourself, Let Level or engine handle creation.
                CharacterFactory.create("MyCharacter") // which returns the type of object.
                So, now when you load in a character, we need a UI that asks what is the class to pick so we create object in memory.

                Also now in .lvl file we need to track the classId of a renderable.

                Does this look reusable ? This way the actual MyCharacter object would only exist in the level.

                Entity based design: ( I am thinking if this looks like a partial replacement to serialization logic itself where scripting is necessary)
                1. create WarriorCharacter.gameobj file which looks like, often called an entity:
                {
                    "classId": "WarriorCharacter",
                    "model_guid": "GUID", // static file
                    "skeleton_guid": "GUID", // static file
                    "statemachine": {
                        "classId": "warriorStateMachine" // states creation, switching on condition all live in this file
                    },
                }
                2. .lvl file has ref to .gameobj
                {
                    "entities": [
                        { "prefab": "Assets/Prefabs/WarriorCharacter.gameobj", "pos": [10, 0, 5] },
                        { "prefab": "Assets/Prefabs/WarriorCharacter.gameobj", "pos": [-5, 0, 2] }
                    ]
                }

                use macros to call this automatically

                #define REGISTER_CHARACTER(ClassName, Key) \
                static struct ClassName##Registrar { \
                    ClassName##Registrar() { \
                        CharacterFactory::Register(Key, []() { \
                            return std::make_unique<ClassName>(); \
                        }); \
                    } \
                } global_##ClassName##Registrar;
 *
 */