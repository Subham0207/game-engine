#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <glm/glm.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/vector.hpp>

namespace Physics
{
    enum class BodyType
    {
        RigidBody = 0,
        SoftBody = 1
    };

    enum class MotionType
    {
        Static = 0,
        Dynamic = 1,
        Kinematic = 2
    };

    enum class ColliderShape
    {
        Box = 0,
        Sphere = 1,
        Capsule = 2,
        Custom = 3
    };

    struct TransformOffset
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);

        template <class Archive>
        void serialize(Archive& ar, const unsigned int /*version*/)
        {
            ar & position;
            ar & rotation;
            ar & scale;
        }
    };

    struct CustomColliderShapeData
    {
        // Cooked geometry only (local-space vertices + triangle indices).
        std::vector<glm::vec3> vertices;
        std::vector<uint32_t> indices;

        [[nodiscard]] bool empty() const
        {
            return vertices.empty() || indices.empty();
        }

        template <class Archive>
        void serialize(Archive& ar, const unsigned int /*version*/)
        {
            ar & vertices;
            ar & indices;
        }
    };

    struct RigidBodyData
    {
        ColliderShape colliderShape = ColliderShape::Box;
        std::optional<CustomColliderShapeData> customColliderShapeData = std::nullopt;
        TransformOffset transformationOffset{};

        template <class Archive>
        void serialize(Archive& ar, const unsigned int /*version*/)
        {
            ar & colliderShape;
            ar & customColliderShapeData;
            ar & transformationOffset;
        }
    };

    struct PhysicsBodySettings
    {
        BodyType bodyType = BodyType::RigidBody;
        MotionType motionType = MotionType::Static;
        RigidBodyData rigidBodyData{};
        // SoftBodyData can be added here later.

        template <class Archive>
        void serialize(Archive& ar, const unsigned int /*version*/)
        {
            ar & bodyType;
            ar & motionType;
            ar & rigidBodyData;
        }
    };
}

