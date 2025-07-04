#include "Physics/capsule.hpp"
#include <EngineState.hpp>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
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
        this->position = position;
        this->rotation = rotation;
        // if(shouldAddToLevel)
        // {
        //     AddToLevel();
        // }
}

void Physics::Capsule::syncTransformation()
{
    //Get capsule dimensions from the 3d mesh
    position = model->GetPosition();
    rotation = model->GetRot();

    glm::quat glmRot = glm::normalize(rotation);
    JPH::Quat jphRotation(glmRot.x, glmRot.y, glmRot.z, glmRot.w);

    // Convert glm::vec3 to JPH::Vec3
    JPH::Vec3 jphPosition(position.x, position.y, position.z);

    physics->RemoveBody(physicsId);

    physicsId = physics->AddCapsule(jphPosition, jphRotation, halfHeight, radius, true);
}
void Physics::Capsule::addCustomModel(std::string modelPath)
{
    capsule = new CapsuleColliderModel();
    getActiveLevel().addRenderable(capsule);
    model = capsule->model;
}

void Physics::Capsule::movebody(float x, float y, float z, float deltaTime, glm::vec3 characterCurrentPos)
{
    static JPH::RVec3 verticalVelocity = JPH::RVec3(0,0,0);

    JPH::RVec3 characterPos = physics->GetPhysicsBodyInterface().GetPosition(physicsId);

    auto currentRotation = physics->GetPhysicsBodyInterface().GetRotation(physicsId);
    JPH::RefConst<JPH::Shape> shape = physics->GetPhysicsBodyInterface().GetShape(physicsId);
    float gravity = -9.81f;
    
    auto velocity = JPH::RVec3(x, y, z);
    verticalVelocity += JPH::RVec3(0, gravity * deltaTime, 0);
    auto totalVelocity = velocity + verticalVelocity;

    auto displacement = totalVelocity * deltaTime;
    auto newPosition = characterPos + displacement;

    JPH::RShapeCast shapeCast(
        shape,
        JPH::Vec3::sReplicate(1.0f), // scale
        JPH::RMat44::sRotationTranslation(currentRotation, characterPos),
        displacement
    );

    JPH::ShapeCastSettings settings;
    settings.mUseShrunkenShapeAndConvexRadius = true;
    settings.mReturnDeepestPoint = true;

    CastShapeClosestHitCollisionCollector collector;
    physics->physicsSystem.GetNarrowPhaseQuery().CastShape(
        shapeCast,                           // Shape cast info
        settings,                           // Cast settings
        characterPos,                       // Base offset for precision
        collector                          // Result collector
        );
    
    if (collector.HadHit())
    {
        const JPH::ShapeCastResult &hit = collector.mHit;
        float fraction = hit.mFraction;
        JPH::Vec3 n = -hit.mPenetrationAxis.Normalized();

        bool grounded = hit.mFraction < 1.0f && n.Dot(JPH::Vec3::sAxisY()) > 0.6f;
        if (grounded)
        {
            // Snap to the hit point and cancel gravity
            newPosition     = characterPos + displacement * hit.mFraction;
            float skin = hit.mPenetrationDepth;
            newPosition += n * (skin + 0.001f);
            verticalVelocity = JPH::RVec3::sZero();
            std::cout << "Hit ground" << std::endl;
        }
    }

    physics->GetPhysicsBodyInterface().MoveKinematic(physicsId, newPosition, currentRotation, deltaTime);
}
void Physics::Capsule::reInit(float radius, float halfheight)
{
    //The reinit does not happen during play; So we only need to update the Model geometry and not collider.
    this->radius = radius;
    this->halfHeight = halfHeight;
    capsule->reGenerateCapsuleColliderMesh(radius, halfHeight);
    model = capsule->model;
}