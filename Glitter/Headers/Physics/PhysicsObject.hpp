#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <PhysicsSystem.hpp>
#include <Physics/PhysicsBodySettings.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace Physics
{
    class PhysicsObject {
    public:
        enum class RuntimeShape
        {
            Box,
            Sphere,
            Capsule,
            Custom
        };

        PhysicsObject() = default;
        PhysicsObject(
            PhysicsSystemWrapper* physics,
            bool isDynamic = false,
            glm::vec3 position = glm::vec3(0.0f),
            glm::quat rotation = glm::quat(),
            glm::vec3 scale = glm::vec3(1.0f)
        );
        virtual ~PhysicsObject() = default;

        virtual void PhysicsUpdate();
        virtual void syncTransformation(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

        void setRuntimeShape(RuntimeShape shape);
        void setMotionType(Physics::MotionType motionType);
        void setTransformOffset(const Physics::TransformOffset& offset);
        void setBoxBaseHalfExtents(const glm::vec3& halfExtents);
        void setCustomColliderGeometry(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices);

        [[nodiscard]] const Physics::TransformOffset& getTransformOffset() const { return transformOffset; }
        [[nodiscard]] Physics::MotionType getMotionType() const { return motionType; }

        void destroyBody();

        [[nodiscard]] glm::vec3 getWorldPosition() const;
        [[nodiscard]] glm::quat getWorldRotation() const;

        PhysicsSystemWrapper* physics;
        JPH::BodyID physicsId;
    private:
        RuntimeShape runtimeShape = RuntimeShape::Box;
        Physics::MotionType motionType = Physics::MotionType::Static;
        Physics::TransformOffset transformOffset{};
        glm::vec3 boxBaseHalfExtents = glm::vec3(1.0f);
        std::vector<glm::vec3> customVertices;
        std::vector<uint32_t> customIndices;
    };
}