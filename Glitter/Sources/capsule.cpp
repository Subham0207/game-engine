#include "Physics/capsule.hpp"
#include <EngineState.hpp>

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

void Physics::Capsule::reInit(float radius, float halfheight)
{
    //The reinit does not happen during play; So we only need to update the Model geometry and not collider.
    this->radius = radius;
    this->halfHeight = halfHeight;
    capsule->reGenerateCapsuleColliderMesh(radius, halfHeight);
    model = capsule->model;
}