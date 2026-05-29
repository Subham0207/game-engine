#include <Physics/PhysicsObject.hpp>

#include <cmath>




Physics::PhysicsObject::PhysicsObject(
    PhysicsSystemWrapper *physics,
    bool isDynamic,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
)
{
    this->physics = physics;
    this->motionType = isDynamic ? Physics::MotionType::Dynamic : Physics::MotionType::Static;
}

void Physics::PhysicsObject::PhysicsUpdate()
{
}

void Physics::PhysicsObject::setRuntimeShape(const RuntimeShape shape)
{
    runtimeShape = shape;
}

void Physics::PhysicsObject::setMotionType(const Physics::MotionType newMotionType)
{
    motionType = newMotionType;
}

void Physics::PhysicsObject::setTransformOffset(const Physics::TransformOffset& offset)
{
    transformOffset = offset;
}

void Physics::PhysicsObject::setBoxBaseHalfExtents(const glm::vec3& halfExtents)
{
    boxBaseHalfExtents = glm::max(halfExtents, glm::vec3(0.01f));
}

void Physics::PhysicsObject::setCustomColliderGeometry(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices)
{
    customVertices = vertices;
    customIndices = indices;
}

void Physics::PhysicsObject::destroyBody()
{
    if (physics)
    {
        physics->RemoveBody(physicsId);
    }
}

void Physics::PhysicsObject::syncTransformation(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
{
    if (!physics)
    {
        return;
    }

    auto motionTypeToJolt = [](const Physics::MotionType type)
    {
        switch (type)
        {
            case Physics::MotionType::Dynamic:
                return JPH::EMotionType::Dynamic;
            case Physics::MotionType::Kinematic:
                return JPH::EMotionType::Kinematic;
            case Physics::MotionType::Static:
            default:
                return JPH::EMotionType::Static;
        }
    };

    const glm::quat offsetQuat = glm::quat(glm::radians(transformOffset.rotation));
    const glm::quat worldRotationGlm = glm::normalize(rotation * offsetQuat);

    const glm::vec3 scaledOffsetPosition = transformOffset.position * scale;
    const glm::vec3 worldPositionGlm = position + (rotation * scaledOffsetPosition);
    const glm::vec3 worldScale = scale * transformOffset.scale;

    const JPH::Quat jphRotation(worldRotationGlm.x, worldRotationGlm.y, worldRotationGlm.z, worldRotationGlm.w);
    const JPH::Vec3 jphPosition(worldPositionGlm.x, worldPositionGlm.y, worldPositionGlm.z);
    const JPH::EMotionType joltMotionType = motionTypeToJolt(motionType);

    physics->RemoveBody(physicsId);

    switch (runtimeShape)
    {
        case RuntimeShape::Sphere:
        {
            const float maxScale = std::max(worldScale.x, std::max(worldScale.y, worldScale.z));
            const float radius = std::max(0.05f, maxScale * 0.5f);
            physicsId = physics->AddSphere(jphPosition, radius, joltMotionType);
            break;
        }
        case RuntimeShape::Capsule:
        {
            const float radius = std::max(0.05f, std::min(worldScale.x, worldScale.z) * 0.5f);
            const float halfHeight = std::max(0.05f, (worldScale.y * 0.5f) - radius);
            physicsId = physics->AddCapsule(jphPosition, jphRotation, halfHeight, radius, joltMotionType);
            break;
        }
        case RuntimeShape::Custom:
        {
            if (joltMotionType != JPH::EMotionType::Static || customVertices.empty() || customIndices.size() < 3)
            {
                physicsId = JPH::BodyID();
                break;
            }

            JPH::VertexList joltVertices;
            joltVertices.reserve(customVertices.size());
            for (const auto& v : customVertices)
            {
                const glm::vec3 scaled = v * worldScale;
                joltVertices.push_back(JPH::Float3(scaled.x, scaled.y, scaled.z));
            }

            JPH::IndexedTriangleList joltTriangles;
            joltTriangles.reserve(customIndices.size() / 3);
            for (size_t i = 0; i + 2 < customIndices.size(); i += 3)
            {
                joltTriangles.push_back(JPH::IndexedTriangle(customIndices[i], customIndices[i + 1], customIndices[i + 2], 0));
            }

            physicsId = physics->AddStaticMesh(jphPosition, jphRotation, joltVertices, joltTriangles);
            break;
        }
        case RuntimeShape::Box:
        default:
        {
            JPH::Vec3 jphHalfExtents(
                std::max(0.05f, std::abs(boxBaseHalfExtents.x * worldScale.x)),
                std::max(0.05f, std::abs(boxBaseHalfExtents.y * worldScale.y)),
                std::max(0.05f, std::abs(boxBaseHalfExtents.z * worldScale.z))
            );
            physicsId = physics->AddBox(jphPosition, jphRotation, jphHalfExtents, joltMotionType);
            break;
        }
    }
}

glm::vec3 Physics::PhysicsObject::getWorldPosition() const
{
    const auto transform = physics->GetBodyPosition(physicsId);
    return glm::vec3(
        static_cast<float>(transform.GetX()),
        static_cast<float>(transform.GetY()),
        static_cast<float>(transform.GetZ())
    );
}

glm::quat Physics::PhysicsObject::getWorldRotation() const
{
    const auto rotation = physics->GetBodyRotation(physicsId);
    return glm::quat(
        static_cast<float>(rotation.GetW()),
        static_cast<float>(rotation.GetX()),
        static_cast<float>(rotation.GetY()),
        static_cast<float>(rotation.GetZ())
    );
}