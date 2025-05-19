#pragma once
#include <PhysicsSystem.hpp>
#include <Physics/PhysicsObject.hpp>

namespace Physics
{
    class Box: public PhysicsObject {
    public:
        Box(
            PhysicsSystemWrapper* physics,
            bool isDynamic = false,
            bool shouldAddToLevel = false,
            glm::vec3 position = glm::vec3(0.0f),
            glm::quat rotation = glm::quat(),
            glm::vec3 scale = glm::vec3(1.0f)
        );
    };
}