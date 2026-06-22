#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <PhysicsSystem.hpp>
#include <Physics/PhysicsBodySettings.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

class Renderable;

namespace Physics
{
    struct RuntimeDynamicsProperties
    {
        float mass = 1.0f;
        bool overrideMass = false;
        glm::vec3 centerOfMassOffset = glm::vec3(0.0f);
        float friction = 0.2f;
        float restitution = 0.0f;
        float linearDamping = 0.05f;
        float angularDamping = 0.05f;
    };

    class PhysicsObject {
    public:
        enum class RuntimeShape
        {
            Box,
            Sphere,
            Capsule,
            Custom,
            ConvexHull
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
        void setPhysicsLayerName(const std::string& layerName);
        void setIsSensor(bool sensor);
        void setDynamicsProperties(const RuntimeDynamicsProperties& properties);
        void setOwnerRenderable(Renderable* owner, const std::string& ownerInstanceId);
        void setBoxBaseHalfExtents(const glm::vec3& halfExtents);
        void setCustomColliderGeometry(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices);

        [[nodiscard]] const Physics::TransformOffset& getTransformOffset() const { return transformOffset; }
        [[nodiscard]] Physics::MotionType getMotionType() const { return motionType; }
        [[nodiscard]] const std::string& getPhysicsLayerName() const { return physicsLayerName; }
        [[nodiscard]] bool getIsSensor() const { return isSensor; }
        [[nodiscard]] Renderable* getOwnerRenderable() const { return ownerRenderable; }
        [[nodiscard]] const std::string& getOwnerInstanceId() const { return ownerInstanceId; }

        void destroyBody();

        [[nodiscard]] glm::vec3 getWorldPosition() const;
        [[nodiscard]] glm::quat getWorldRotation() const;

        void MoveBody(const glm::vec3& position, float deltaTime) const;
        void MoveBody(const glm::vec3& position, const glm::quat& rotation, float deltaTime) const;

        PhysicsSystemWrapper* physics;
        JPH::BodyID physicsId;
    private:
        RuntimeShape runtimeShape = RuntimeShape::Box;
        Physics::MotionType motionType = Physics::MotionType::Static;
        Physics::TransformOffset transformOffset{};
        std::string physicsLayerName = "Default";
        bool isSensor = false;
        RuntimeDynamicsProperties dynamicsProperties{};
        glm::vec3 boxBaseHalfExtents = glm::vec3(1.0f);
        std::vector<glm::vec3> customVertices;
        std::vector<uint32_t> customIndices;
        Renderable* ownerRenderable = nullptr;
        std::string ownerInstanceId;
    };
}