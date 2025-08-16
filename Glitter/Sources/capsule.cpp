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
        // model->setTransform(positison, rotation, scale);
        // this->position = position;
        // this->rotation = rotation;
        // if(shouldAddToLevel)
        // {
        //     AddToLevel();
        // }
        set = nullptr;
        // m_temp =
        // std::make_unique<JPH::TempAllocatorImpl>(64 * 1024);
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

void Physics::Capsule::movebody(float x, float y, float z, float deltaTime, glm::vec3 characterCurrentPos, glm::quat glmYaw)
{
    JPH::TempAllocatorImpl temp(64 * 1024);
    using namespace JPH;
    Vec3 input_move = Vec3(x,y,z);
    bool want_jump = false;
    auto system = &physics->physicsSystem;

    const Vec3 kGravity      = Vec3(0, -9.81f, 0);
    const float kWalkSpeed   = 4.0f;
    const float kJumpSpeed   = 5.0f;

    Vec3 v = character->GetGroundVelocity();
    v += input_move * kWalkSpeed;

    if (character->GetGroundState() == CharacterBase::EGroundState::OnGround)
    {
        if (want_jump)
            v += kJumpSpeed * character->GetUp();
    }
    else
    {
        v += kGravity;
    }

    character->SetLinearVelocity(v);

    //std::cout<< "Character location: " << character->GetPosition().GetX() << " " << character->GetPosition().GetY() << " " << character->GetPosition().GetZ() << std::endl;

    JPH::Quat joltYaw(glmYaw.x, glmYaw.y, glmYaw.z, glmYaw.w);

    character->SetRotation(joltYaw);

    CharacterVirtual::ExtendedUpdateSettings eus;
    character->ExtendedUpdate(deltaTime,
                                kGravity,
                                eus,
                                {},
                                {},
                                {}, {}, temp);


    grounded     = character->GetGroundState() == CharacterBase::EGroundState::OnGround;
    ground_normal= character->GetGroundNormal();
    landed       = listener->has_landed_this_frame;
    listener->has_landed_this_frame = false;

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