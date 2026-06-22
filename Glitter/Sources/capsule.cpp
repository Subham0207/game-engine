#include "Physics/capsule.hpp"
#include <Character/Character.hpp>
#include <EngineState.hpp>
#include <Physics/PhysicsLayerRegistry.hpp>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <algorithm>
#include <limits>
using CastShapeClosestHitCollisionCollector = JPH::ClosestHitCollisionCollector<JPH::CastShapeCollector>;

namespace
{
    JPH::CharacterVsCharacterCollisionSimple& getCharacterVsCharacterCollisionRegistry()
    {
        static JPH::CharacterVsCharacterCollisionSimple registry;
        return registry;
    }
}

Physics::Capsule::Capsule
    (PhysicsSystemWrapper *physics,
    float radius,
    float halfHeight,
    bool isDynamic,
    bool shouldAddToLevel,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale) : PhysicsObject(
        physics,
        isDynamic,
        position,
        rotation,
        scale
    )
{
    set = nullptr;
    this->mRadius = radius;
    this->mHalfHeight = halfHeight;

    JPH::Vec3 jphPosition(position.x, position.y, position.z);
    CreateCharacterVirtualPhysics(&physics->physicsSystem,
    jphPosition, halfHeight, radius);
}

Physics::Capsule::~Capsule()
{
    if (character)
    {
        if (physics)
        {
            physics->UnregisterBodyOwner(character->GetInnerBodyID());
        }
        character->SetListener(nullptr);
        getCharacterVsCharacterCollisionRegistry().Remove(character);
        delete character;
        character = nullptr;
    }

    if (set)
    {
        set->Release();
        set = nullptr;
    }

    delete listener;
    listener = nullptr;
}

void Physics::Capsule::syncTransformation()
{
    const auto worldPosition = getWorldPosition();
    JPH::Vec3 jphPosition(worldPosition.x, worldPosition.y, worldPosition.z);

    if(!set)
    {
        CreateCharacterVirtualPhysics(&physics->physicsSystem,
            jphPosition, mHalfHeight, mRadius);
    }
    else
    {
        if (character) {
            if (physics)
            {
                physics->UnregisterBodyOwner(character->GetInnerBodyID());
            }
            character->SetListener(nullptr);
            getCharacterVsCharacterCollisionRegistry().Remove(character);
            delete character;
            character = nullptr;
        }
        set->Release();
        delete listener;
        CreateCharacterVirtualPhysics(&physics->physicsSystem,
            jphPosition, mHalfHeight, mRadius);

    }
}

void Physics::Capsule::moveBody(
    float deltaTime,
    glm::vec3 moveOffset,
    glm::quat rotationOffset,
    bool& want_jump,
    float movementSpeed,
    float jumpSpeed
    )
{
    using namespace JPH;
    TempAllocatorImpl temp(64 * 1024);
    listener->beginFrame();

    // Choose sane units (meters). Tune from here if your world is scaled.
    const Vec3 kGravity = Vec3(0.0f, -9.81f, 0.0f);

    // Desired horizontal velocity from input (x,z). Keep y = 0
    Vec3 desired_horizontal = Vec3(moveOffset.x, moveOffset.y, moveOffset.z) * movementSpeed;

    Vec3 v{};

    if (character->GetGroundState() == CharacterBase::EGroundState::OnGround)
    {
        // Start from ground’s velocity only while supported
        v = character->GetGroundVelocity() + desired_horizontal;

        if (want_jump == true)
        {
            // Add an instant vertical impulse along the up axis once
            v += jumpSpeed * character->GetUp();
            want_jump = false;
        }
    }
    else
    {
        // In air: keep the *current* vertical velocity, add horizontal + gravity
        v = character->GetLinearVelocity();
        v.SetX(desired_horizontal.GetX());
        v.SetZ(desired_horizontal.GetZ());
    }

    v += kGravity * deltaTime;
    character->SetLinearVelocity(v);

    // Rotation handoff (GLM wxyz -> Jolt xyzw ctor)
    JPH::Quat rotOffset(rotationOffset.x, rotationOffset.y, rotationOffset.z, rotationOffset.w);
    rotOffset = rotOffset.Normalized();
    assert(!rotOffset.IsNaN() && "Jolt Quaternion became NaN after normalization!");
    character->SetRotation(rotOffset);

    CharacterVirtual::ExtendedUpdateSettings eus;
    const JPH::ObjectLayer capsuleLayer = Physics::PhysicsLayerRegistry::instance().getLayerOrDefault(getPhysicsLayerName());
    const JPH::DefaultBroadPhaseLayerFilter broadPhaseLayerFilter = physics->physicsSystem.GetDefaultBroadPhaseLayerFilter(capsuleLayer);
    const JPH::DefaultObjectLayerFilter objectLayerFilter = physics->physicsSystem.GetDefaultLayerFilter(capsuleLayer);
    const JPH::BodyFilter bodyFilter;
    const JPH::ShapeFilter shapeFilter;
    character->ExtendedUpdate(deltaTime, kGravity, eus, broadPhaseLayerFilter, objectLayerFilter, bodyFilter, shapeFilter, temp);

    grounded      = character->GetGroundState() == CharacterBase::EGroundState::OnGround;
    ground_normal = character->GetGroundNormal();
    landed        = listener->has_landed_this_frame; // if you track it
    character->UpdateGroundVelocity();

    if (character->GetPosition().IsNaN()) {
        // RESET character to a safe position or previous frame position
        character->SetPosition(JPH::Vec3(0, 10, 0));
        character->SetLinearVelocity(JPH::Vec3::sZero());
        assert(false && "Jolt Character went NaN!");
    }

    // CharacterVirtual collisions don't flow through the rigid-body contact listener,
    // so forward them to PhysicsSystem's collision recording path.
    if (physics != nullptr && character != nullptr)
    {
        const uint32_t selfCharacterId = getCharacterId();
        if (selfCharacterId != std::numeric_limits<uint32_t>::max())
        {
            const JPH::BodyID selfBodyId = character->GetInnerBodyID();
            for (const auto& [otherCapsuleId, points] : listener->getCharacterContactPoints())
            {
                if (otherCapsuleId == std::numeric_limits<uint32_t>::max() || selfCharacterId > otherCapsuleId)
                {
                    continue;
                }

                Character* otherCharacter = Character::getCharacterByCapsuleId(otherCapsuleId);
                if (otherCharacter == nullptr || otherCharacter->capsuleCollider == nullptr ||
                    otherCharacter->capsuleCollider->character == nullptr)
                {
                    continue;
                }

                physics->recordBodyCollision(selfBodyId, otherCharacter->capsuleCollider->character->GetInnerBodyID(), points);
            }
        }
    }
}

void Physics::Capsule::tick()
{
}

void Physics::Capsule::reInit(float radius, float halfheight)
{
    // Rebuild the CharacterVirtual shape with the new dimensions.
    this->mRadius = radius;
    this->mHalfHeight = halfheight;

    auto jphPosition = character->GetPosition();

    if (character) {
        if (physics)
        {
            physics->UnregisterBodyOwner(character->GetInnerBodyID());
        }
        character->SetListener(nullptr);
        getCharacterVsCharacterCollisionRegistry().Remove(character);
        delete character;
        character = nullptr;
    }
    set->Release();
    delete listener;

    CreateCharacterVirtualPhysics(&physics->physicsSystem,
    jphPosition, halfheight, radius);
}

void Physics::Capsule::CreateCharacterVirtualPhysics(JPH::PhysicsSystem *system,
            const JPH::RVec3 &spawn, float halfheight, float radius)
{
    // --- Build the settings ------------------------------------------------
    set = new JPH::CharacterVirtualSettings();
    set->mShape = new JPH::CapsuleShape(halfheight, radius);       // two-sphere capsule
    set->mInnerBodyShape = set->mShape;
    set->mInnerBodyLayer = Physics::PhysicsLayerRegistry::instance().getLayerOrDefault(getPhysicsLayerName());
    set->mMaxSlopeAngle     = JPH::DegreesToRadians(55.0f);           // walkable if ≤ 55°
    set->mSupportingVolume  = JPH::Plane(JPH::Vec3::sAxisY(), -radius);
    set->mPredictiveContactDistance = 0.1f;                           // prevents snagging
    set->mMass              = std::max(0.001f, characterMass);
    set->mMaxStrength       = std::max(0.0f, characterMaxStrength);
    // If you need the character to show up in regular overlap queries,
    // give it an “inner” rigid body:
    // set->mInnerBodyShape = set->mShape;   // (optional)

    std::cout<< "Spawn location: " << spawn.GetX() << " " << spawn.GetY() << " " << spawn.GetZ() << std::endl;

    character = new JPH::CharacterVirtual(set, spawn,
                                            JPH::Quat::sIdentity(),
                                            /*userData*/0, system);

    //To enable character vs character collision. And now during Extended update Jolt detects other characters.
    character->SetCharacterVsCharacterCollision(&getCharacterVsCharacterCollisionRegistry());
    getCharacterVsCharacterCollisionRegistry().Add(character);

    listener = new MyContactListener();
    listener->setCharacterCollisionRuleEvaluator([](const uint32_t selfCharacterId, const uint32_t otherCharacterId)
    {
        const Character* selfCharacter = Character::getCharacterByCapsuleId(selfCharacterId);
        const Character* otherCharacter = Character::getCharacterByCapsuleId(otherCharacterId);
        if (selfCharacter == nullptr || otherCharacter == nullptr ||
            selfCharacter->capsuleCollider == nullptr || otherCharacter->capsuleCollider == nullptr)
        {
            return true;
        }

        const auto& layerRegistry = Physics::PhysicsLayerRegistry::instance();
        const JPH::ObjectLayer selfLayer = layerRegistry.getLayerOrDefault(selfCharacter->capsuleCollider->getPhysicsLayerName());
        const JPH::ObjectLayer otherLayer = layerRegistry.getLayerOrDefault(otherCharacter->capsuleCollider->getPhysicsLayerName());
        return Physics::PhysicsLayerRulebookRegistry::instance().shouldCollide(selfLayer, otherLayer);
    });
    character->SetListener(listener);                                  // ground callbacks

    auto body_id = character->GetInnerBodyID();
    if (physics)
    {
        physics->RegisterBodyOwner(body_id, getOwnerRenderable(), getOwnerInstanceId(), getIsSensor());
    }
    auto &lock_interface = physics->physicsSystem.GetBodyLockInterface();
    {
        JPH::BodyLockWrite lock(lock_interface, character->GetInnerBodyID());
        if (lock.Succeeded())
        {
            JPH::Body &body = lock.GetBody();
            body.SetFriction(std::max(0.0f, characterFriction));
            body.SetRestitution(std::max(0.0f, characterRestitution));
            body.SetIsSensor(getIsSensor());
        }
    }
}

glm::mat4 Physics::Capsule::getWorldTransformation() const
{
    const auto transform = character->GetWorldTransform();
    return AssimpHelpers::ConvertMatrixToGLMFormat(transform);
}

glm::vec3 Physics::Capsule::getWorldPosition()
{
    auto pos = character->GetPosition();
    return AssimpHelpers::toGlM(pos);
}

glm::quat Physics::Capsule::getWorldRotation()
{
    auto rot = character->GetRotation();
    return AssimpHelpers::toGlM(rot);
}

glm::vec3 Physics::Capsule::getWorldScale()
{
    return glm::vec3(1.0f, 1.0f, 1.0f);
}

void Physics::Capsule::setWorldPosition(glm::vec3 position)
{
    auto pos = JPH::RVec3(position.x, position.y, position.z);
    character->SetPosition(pos);
}

void Physics::Capsule::setWorldRotation(glm::quat rotation)
{
    auto rot = JPH::Quat(rotation.x, rotation.y, rotation.z, rotation.w);
    character->SetRotation(rot);
}

void Physics::Capsule::PhysicsUpdate()
{
    const auto transform = character->GetPosition();
    const auto transformglm = glm::vec3(static_cast<float>(transform.GetX()), static_cast<float>(transform.GetY()), static_cast<float>(transform.GetZ()));
    assert(!glm::any(glm::isnan(transformglm)) && "Jolt Character Position is NaN!");
}


uint32_t Physics::Capsule::getCharacterId() const
{
    if (character == nullptr)
    {
        return std::numeric_limits<uint32_t>::max();
    }

    return character->GetID().GetValue();
}

