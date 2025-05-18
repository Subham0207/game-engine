#include "Physics/Box.hpp"
#include <glm/glm.hpp>
#include <EngineState.hpp>
#include <Jolt/Physics/PhysicsSystem.h>

void Physics::Box::PhysicsUpdate()
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

void Physics::Box::syncTransformation()
{
    //if transformation changed when physics was disabled
    //Propagate these transformations to physics body
    JPH::BodyInterface& bodyInterface = physics->GetPhysicsBodyInterface();

    auto position = model->GetPosition();
    JPH::Vec3 jphPosition(position.x, position.y, position.z);
    bodyInterface.SetPosition(physicsId, jphPosition,JPH::EActivation::Activate);

    auto glmRot = model->GetRot();
    JPH::Quat jphRot(glmRot.x, glmRot.y, glmRot.z, glmRot.w);
    bodyInterface.SetRotation(physicsId, jphRot, JPH::EActivation::Activate);

    //Scale is immutable
}

void Physics::Box::AddToLevel()
{
    getActiveLevel().addRenderable(model);
    getUIState().renderables = *State::state->activeLevel.renderables;
}

Physics::Box::Box(
    PhysicsSystemWrapper* physics,
    bool isDynamic,
    bool shouldAddToLevel,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
)
{
   model = new Model("E:/OpenGL/Glitter/EngineAssets/cube.fbx");
   this->physics = physics;


    // Convert glm::quat to JPH::Quat
    glm::quat glmRot = glm::normalize(rotation);
    JPH::Quat jphRotation(glmRot.x, glmRot.y, glmRot.z, glmRot.w);

    // Convert glm::vec3 to JPH::Vec3
    JPH::Vec3 jphPosition(position.x, position.y, position.z);
    JPH::Vec3 jphHalfExtents(scale.x, scale.y, scale.z); // BoxShape expects half extents

    // Add physics box
    physicsId = this->physics->AddBox(
        jphPosition,
        jphRotation,
        jphHalfExtents,
        isDynamic
    );

    // Apply transform to the model
    model->setTransform(position, rotation, scale);

    if(shouldAddToLevel)
    {
        AddToLevel();
    }
}