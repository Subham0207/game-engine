#include "Physics/capsule.hpp"
#include <EngineState.hpp>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
using CastShapeClosestHitCollisionCollector = JPH::ClosestHitCollisionCollector<JPH::CastShapeCollector>;

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
        nullptr,
        isDynamic,
        shouldAddToLevel,
        position,
        rotation,
        scale
    )
{
    set = nullptr;
    this->mRadius = radius;
    this->mHalfHeight = halfHeight;
    addCustomModel("");

    JPH::Vec3 jphPosition(position.x, position.y, position.z);
    CreateCharacterVirtualPhysics(&physics->physicsSystem,
    jphPosition, halfHeight, radius);
}

void Physics::Capsule::syncTransformation()
{
    //Get capsule dimensions from the 3d mesh
    auto position = model->GetPosition();
    auto rotation = model->GetRot();

    glm::quat glmRot = glm::normalize(rotation);
    JPH::Quat jphRotation(glmRot.x, glmRot.y, glmRot.z, glmRot.w);

    // Convert glm::vec3 to JPH::Vec3
    JPH::Vec3 jphPosition(position.x, position.y, position.z);

    if(!set)
    {
        CreateCharacterVirtualPhysics(&physics->physicsSystem,
            jphPosition, mHalfHeight, mRadius);
    }
    else
    {
        if (character) {
            character->SetListener(nullptr);
            character = nullptr;            
        }
        set->Release();
        delete listener;
        CreateCharacterVirtualPhysics(&physics->physicsSystem,
            jphPosition, mHalfHeight, mRadius);

    }
}
void Physics::Capsule::addCustomModel(std::string modelPath)
{
    capsule = std::make_shared<CapsuleColliderModel>(mRadius, mHalfHeight);
    getActiveLevel().addRenderable(capsule);
    model = capsule->model;
}

void Physics::Capsule::moveBody(
    float deltaTime,
    glm::vec3 moveOffset,
    glm::quat rotationOffset,
    bool& want_jump,
    float walkSpeed,
    float jumpSpeed
    )
{
    using namespace JPH;
    TempAllocatorImpl temp(64 * 1024);

    // Choose sane units (meters). Tune from here if your world is scaled.
    const Vec3 kGravity = Vec3(0.0f, -9.81f, 0.0f);

    // Desired horizontal velocity from input (x,z). Keep y = 0
    Vec3 desired_horizontal = Vec3(moveOffset.x, moveOffset.y, moveOffset.z) * walkSpeed;

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
    character->ExtendedUpdate(deltaTime, kGravity, eus, {}, {}, {}, {}, temp);

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
}

void Physics::Capsule::tick()
{
    model->setModelMatrix(getWorldTransformation());
}

void Physics::Capsule::reInit(float radius, float halfheight)
{
    //The reinit does not happen during play; So we only need to update the Model geometry and not collider.
    this->mRadius = radius;
    this->mHalfHeight = halfheight;
    capsule->reGenerateCapsuleColliderMesh(radius, halfheight);
    model = capsule->model;

    auto jphPosition = character->GetPosition();

    if (character) {
        character->SetListener(nullptr);
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
    set->mMaxSlopeAngle     = JPH::DegreesToRadians(55.0f);           // walkable if ≤ 55°
    set->mSupportingVolume  = JPH::Plane(JPH::Vec3::sAxisY(), -radius);
    set->mPredictiveContactDistance = 0.1f;                           // prevents snagging
    set->mMass              = 80.0f;
    // If you need the character to show up in regular overlap queries,
    // give it an “inner” rigid body:
    // set->mInnerBodyShape = set->mShape;   // (optional)

    std::cout<< "Spawn location: " << spawn.GetX() << " " << spawn.GetY() << " " << spawn.GetZ() << std::endl;

    character = new JPH::CharacterVirtual(set, spawn,
                                            JPH::Quat::sIdentity(),
                                            /*userData*/0, system);

    listener = new MyContactListener();
    character->SetListener(listener);                                  // ground callbacks

    auto body_id = character->GetInnerBodyID();
    auto &lock_interface = physics->physicsSystem.GetBodyLockInterface();
    {
        JPH::BodyLockWrite lock(lock_interface, character->GetInnerBodyID());
        if (lock.Succeeded())
        {
            JPH::Body &body = lock.GetBody();
            body.SetRestitution(0.0f);
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
    auto transform = character->GetPosition();
    auto transformglm = glm::vec3(static_cast<float>(transform.GetX()), static_cast<float>(transform.GetY()), static_cast<float>(transform.GetZ()));

    assert(!glm::any(glm::isnan(transformglm)) && "Jolt Character Position is NaN!");

    auto rotation = character->GetRotation();
    auto rotationglm = glm::quat(
                        static_cast<float>(rotation.GetW()),
                        static_cast<float>(rotation.GetX()),
                        static_cast<float>(rotation.GetY()),
                        static_cast<float>(rotation.GetZ())
                    );

    assert(!glm::any(glm::isnan(glm::vec4(rotationglm.x, rotationglm.y, rotationglm.z, rotationglm.w)))
           && "Jolt Character Rotation is NaN!");
    model->setTransformFromPhysics(transformglm, rotationglm);
}
