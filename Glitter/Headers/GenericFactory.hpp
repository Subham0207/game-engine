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