#include <Physics/PhysicsObject.hpp>
#include <Physics/PhysicsLayerRegistry.hpp>

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

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

void Physics::PhysicsObject::setPhysicsLayerName(const std::string& layerName)
{
    physicsLayerName = layerName.empty() ? "Default" : layerName;
}

void Physics::PhysicsObject::setIsSensor(const bool sensor)
{
    isSensor = sensor;
}

void Physics::PhysicsObject::setDynamicsProperties(const RuntimeDynamicsProperties& properties)
{
    dynamicsProperties = properties;
}

void Physics::PhysicsObject::setOwnerRenderable(Renderable* owner, const std::string& instanceId)
{
    ownerRenderable = owner;
    ownerInstanceId = instanceId;
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
        physics->UnregisterBodyOwner(physicsId);
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
    const JPH::ObjectLayer objectLayer = Physics::PhysicsLayerRegistry::instance().getLayerOrDefault(physicsLayerName);

    physics->UnregisterBodyOwner(physicsId);
    physics->RemoveBody(physicsId);

    JPH::RefConst<JPH::Shape> bodyShape;

    switch (runtimeShape)
    {
        case RuntimeShape::Sphere:
        {
            const float baseRadius = std::max(
                0.01f,
                std::max(boxBaseHalfExtents.x, std::max(boxBaseHalfExtents.y, boxBaseHalfExtents.z)));
            const float maxScale = std::max(std::abs(worldScale.x), std::max(std::abs(worldScale.y), std::abs(worldScale.z)));
            const float radius = std::max(0.05f, baseRadius * maxScale);
            bodyShape = new JPH::SphereShape(radius);
            break;
        }
        case RuntimeShape::Capsule:
        {
            const float baseRadius = std::max(0.01f, std::min(boxBaseHalfExtents.x, boxBaseHalfExtents.z));
            const float baseHalfHeight = std::max(0.01f, boxBaseHalfExtents.y - baseRadius);
            const float radiusScale = std::min(std::abs(worldScale.x), std::abs(worldScale.z));
            const float halfHeightScale = std::abs(worldScale.y);
            const float radius = std::max(0.05f, baseRadius * radiusScale);
            const float halfHeight = std::max(0.05f, baseHalfHeight * halfHeightScale);
            bodyShape = new JPH::CapsuleShape(halfHeight, radius);
            break;
        }
        case RuntimeShape::Custom:
        {
            if (joltMotionType != JPH::EMotionType::Static || customVertices.empty() || customIndices.size() < 3)
            {
                physicsId = JPH::BodyID();
                return;
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

            JPH::MeshShapeSettings meshSettings(std::move(joltVertices), std::move(joltTriangles));
            JPH::ShapeSettings::ShapeResult shapeResult = meshSettings.Create();
            if (shapeResult.HasError())
            {
                physicsId = JPH::BodyID();
                return;
            }

            bodyShape = shapeResult.Get();
            break;
        }
        case RuntimeShape::ConvexHull:
        {
            if (customVertices.empty())
            {
                physicsId = JPH::BodyID();
                return;
            }

            JPH::Array<JPH::Vec3> hullPoints;
            hullPoints.reserve(customVertices.size());
            for (const auto& v : customVertices)
            {
                const glm::vec3 scaled = v * worldScale;
                hullPoints.emplace_back(scaled.x, scaled.y, scaled.z);
            }

            JPH::ConvexHullShapeSettings hullSettings(hullPoints);
            JPH::ShapeSettings::ShapeResult shapeResult = hullSettings.Create();
            if (shapeResult.HasError())
            {
                physicsId = JPH::BodyID();
                return;
            }

            bodyShape = shapeResult.Get();

            // Convex hull COM is computed from geometry and is often not at model origin.
            // Recenter so render transform and physics body align across orientations.
            const JPH::Vec3 hullCenterOfMass = bodyShape->GetCenterOfMass();
            if (hullCenterOfMass.LengthSq() > 1.0e-8f)
            {
                bodyShape = new JPH::OffsetCenterOfMassShape(bodyShape, -hullCenterOfMass);
            }
            break;
        }
        case RuntimeShape::Box:
        default:
        {
            const JPH::Vec3 jphHalfExtents(
                std::max(0.05f, std::abs(boxBaseHalfExtents.x * worldScale.x)),
                std::max(0.05f, std::abs(boxBaseHalfExtents.y * worldScale.y)),
                std::max(0.05f, std::abs(boxBaseHalfExtents.z * worldScale.z))
            );
            bodyShape = new JPH::BoxShape(jphHalfExtents);
            break;
        }
    }

    if (bodyShape == nullptr)
    {
        physicsId = JPH::BodyID();
        return;
    }

    if (glm::length(dynamicsProperties.centerOfMassOffset) > 0.0001f)
    {
        const JPH::Vec3 comOffset(
            dynamicsProperties.centerOfMassOffset.x,
            dynamicsProperties.centerOfMassOffset.y,
            dynamicsProperties.centerOfMassOffset.z);
        bodyShape = new JPH::OffsetCenterOfMassShape(bodyShape, comOffset);
    }

    JPH::BodyCreationSettings bodySettings(
        bodyShape,
        jphPosition,
        jphRotation,
        joltMotionType,
        objectLayer);
    bodySettings.mIsSensor = isSensor;
    bodySettings.mFriction = dynamicsProperties.friction;
    bodySettings.mRestitution = dynamicsProperties.restitution;
    bodySettings.mLinearDamping = dynamicsProperties.linearDamping;
    bodySettings.mAngularDamping = dynamicsProperties.angularDamping;

    if (dynamicsProperties.overrideMass && joltMotionType == JPH::EMotionType::Dynamic)
    {
        bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        bodySettings.mMassPropertiesOverride.mMass = std::max(0.001f, dynamicsProperties.mass);
    }

    physicsId = physics->AddBody(bodySettings);

    if (physics != nullptr)
    {
        physics->RegisterBodyOwner(physicsId, ownerRenderable, ownerInstanceId, isSensor);
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

void Physics::PhysicsObject::MoveBody(const glm::vec3& position, const float deltaTime) const
{
    //TODO: Maybe add motion type == kinematic check
    physics->GetPhysicsBodyInterface().MoveKinematic(
        physicsId,
        JPH::Vec3(position.x, position.y, position.z),
        physics->GetBodyRotation(physicsId),
        deltaTime);
}

void Physics::PhysicsObject::MoveBody(const glm::vec3& position, const glm::quat& rotation, const float deltaTime) const
{
    physics->GetPhysicsBodyInterface().MoveKinematic(
        physicsId,
        JPH::Vec3(position.x, position.y, position.z),
        JPH::Quat(rotation.x, rotation.y, rotation.z, rotation.w),
        deltaTime);
}
