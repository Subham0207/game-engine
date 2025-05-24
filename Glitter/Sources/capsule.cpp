#include "Physics/capsule.hpp"

Physics::Capsule::Capsule
    (PhysicsSystemWrapper *physics,
    bool isDynamic,
    bool shouldAddToLevel,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale) : PhysicsObject(
        physics,
        "E:/OpenGL/Glitter/EngineAssets/cube.fbx",
        isDynamic,
        shouldAddToLevel,
        position,
        rotation,
        scale
    )
{
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

    physics->RemoveBody(physicsId);

    physicsId = physics->AddCapsule(jphPosition, jphRotation, halfHeight, radius, true);
}