#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <PhysicsSystem.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Physics
{
    class PhysicsObject {
    public:
        PhysicsObject() = default;
        PhysicsObject(
            PhysicsSystemWrapper* physics,
            bool isDynamic = false,
            glm::vec3 position = glm::vec3(0.0f),
            glm::quat rotation = glm::quat(),
            glm::vec3 scale = glm::vec3(1.0f)
        );
        virtual void PhysicsUpdate();
        virtual void syncTransformation(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);
        [[nodiscard]] glm::vec3 getWorldPosition() const;
        [[nodiscard]] glm::quat getWorldRotation() const;
        PhysicsSystemWrapper* physics;
        JPH::BodyID physicsId;
    private:
        bool isDynamic;
    };
}