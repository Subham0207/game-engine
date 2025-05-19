#include <Physics/PhysicsObject.hpp>
#include <EngineState.hpp>

Physics::PhysicsObject::PhysicsObject(
    PhysicsSystemWrapper *physics,
    const char* modelPath,
    bool isDynamic,
    bool shouldAddToLevel,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
)
{
    model = new Model(modelPath);
    this->physics = physics;
    this->isDynamic = isDynamic;
    model->setTransform(position, rotation, scale);

    if(shouldAddToLevel)
    {
        AddToLevel();
    }
}

void Physics::PhysicsObject::PhysicsUpdate()
{
    auto transform = physics->GetBodyPosition(physicsId);
    auto transformglm = glm::vec3(static_cast<float>(transform.GetX()), static_cast<float>(transform.GetY()), static_cast<float>(transform.GetZ()));

    auto rotation = physics->GetBodyRotation(physicsId);
    auto rotationglm = glm::quat(
                        static_cast<float>(rotation.GetW()),
                        static_cast<float>(rotation.GetX()),
                        static_cast<float>(rotation.GetY()),
                        static_cast<float>(rotation.GetZ())
                    );

    model->setTransformFromPhysics(transformglm, rotationglm);
}

void Physics::PhysicsObject::syncTransformation()
{
    auto position = model->GetPosition();
    auto scale = model->GetScale();
    auto rotation = model->GetRot();

    glm::quat glmRot = glm::normalize(rotation);
    JPH::Quat jphRotation(glmRot.x, glmRot.y, glmRot.z, glmRot.w);

    // Convert glm::vec3 to JPH::Vec3
    JPH::Vec3 jphPosition(position.x, position.y, position.z);
    JPH::Vec3 jphHalfExtents(scale.x, scale.y, scale.z); // BoxShape expects half extents

    physics->RemoveBody(physicsId);

    // Add physics box
    physicsId = this->physics->AddBox(
        jphPosition,
        jphRotation,
        jphHalfExtents,
        isDynamic
    );
}

void Physics::PhysicsObject::AddToLevel()
{
    getActiveLevel().addRenderable(model);
    getUIState().renderables = *State::state->activeLevel.renderables;
}