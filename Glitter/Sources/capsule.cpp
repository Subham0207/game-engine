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
        addCustomModel("");
        set = nullptr;
        this->radius = radius;
        this->halfHeight = halfHeight;
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
            jphPosition, halfHeight, radius);
    }
    else
    {
        delete set;
        delete character;
        delete listener;
        CreateCharacterVirtualPhysics(&physics->physicsSystem,
            jphPosition, halfHeight, radius);

    }
}
void Physics::Capsule::addCustomModel(std::string modelPath)
{
    capsule = new CapsuleColliderModel();
    getActiveLevel().addRenderable(capsule);
    model = capsule->model;
}

void Physics::Capsule::movebody(float x, float y, float z, float deltaTime, glm::vec3 characterCurrentPos, glm::quat glmYaw, bool& want_jump, float walkSpeed)
{
    using namespace JPH;
    TempAllocatorImpl temp(64 * 1024);

    // Choose sane units (meters). Tune from here if your world is scaled.
    const Vec3 kGravity = Vec3(0.0f, -9.81f, 0.0f);
    const float kJumpSpeed = 6.0f;     // m/s (try 4–8 first, not 900)

    // Desired horizontal velocity from input (x,z). Keep y = 0
    Vec3 desired_horizontal = Vec3(x, 0.0f, z) * walkSpeed;

    Vec3 v;

    if (character->GetGroundState() == CharacterBase::EGroundState::OnGround)
    {
        // Start from ground’s velocity only while supported
        v = character->GetGroundVelocity() + desired_horizontal;

        if (want_jump == true)
        {
            // Add an instant vertical impulse along the up axis once
            v += kJumpSpeed * character->GetUp();
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
    JPH::Quat joltYaw(glmYaw.x, glmYaw.y, glmYaw.z, glmYaw.w);
    character->SetRotation(joltYaw);

    CharacterVirtual::ExtendedUpdateSettings eus;
    character->ExtendedUpdate(deltaTime, kGravity, eus, {}, {}, {}, {}, temp);

    grounded      = character->GetGroundState() == CharacterBase::EGroundState::OnGround;
    ground_normal = character->GetGroundNormal();
    landed        = listener->has_landed_this_frame; // if you track it
    character->UpdateGroundVelocity();
}
void Physics::Capsule::reInit(float radius, float halfheight)
{
    //The reinit does not happen during play; So we only need to update the Model geometry and not collider.
    this->radius = radius;
    this->halfHeight = halfHeight;
    capsule->reGenerateCapsuleColliderMesh(radius, halfHeight);
    model = capsule->model;
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

void Physics::Capsule::PhysicsUpdate()
{
    auto transform = character->GetPosition();
    auto transformglm = glm::vec3(static_cast<float>(transform.GetX()), static_cast<float>(transform.GetY()), static_cast<float>(transform.GetZ()));

    auto rotation = character->GetRotation();
    auto rotationglm = glm::quat(
                        static_cast<float>(rotation.GetW()),
                        static_cast<float>(rotation.GetX()),
                        static_cast<float>(rotation.GetY()),
                        static_cast<float>(rotation.GetZ())
                    );

    model->setTransformFromPhysics(transformglm, rotationglm);
}