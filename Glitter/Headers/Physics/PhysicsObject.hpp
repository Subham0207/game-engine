#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <PhysicsSystem.hpp>
#include <3DModel/model.hpp>

namespace Physics
{
    class PhysicsObject {
    public:
        PhysicsObject(
            PhysicsSystemWrapper* physics,
            const char* modelPath,
            bool isDynamic = false,
            bool shouldAddToLevel = false,
            glm::vec3 position = glm::vec3(0.0f),
            glm::quat rotation = glm::quat(),
            glm::vec3 scale = glm::vec3(1.0f)
        );
        virtual void PhysicsUpdate();
        virtual void syncTransformation();
        virtual void addCustomModel(std::string modelPath);
        void AddToLevel();
        Model * model;
        PhysicsSystemWrapper* physics;
        JPH::BodyID physicsId;
    private:
        bool isDynamic;
    };
}