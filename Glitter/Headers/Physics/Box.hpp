#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <PhysicsSystem.hpp>
#include <3DModel/model.hpp>

namespace Physics
{
    class Box {
    public:
        Box(
            PhysicsSystemWrapper* physics,
            bool isDynamic = false,
            bool shouldAddToLevel = false,
            glm::vec3 position = glm::vec3(0.0f),
            glm::quat rotation = glm::quat(),
            glm::vec3 scale = glm::vec3(1.0f)
        );
        void PhysicsUpdate();
        void syncTransformation();
        void AddToLevel();
    private:
        JPH::BodyID physicsId;
        PhysicsSystemWrapper* physics;
        Model * model;
    };
}