#include <Physics/PhysicsObject.hpp>


Physics::PhysicsObject::PhysicsObject(
    PhysicsSystemWrapper *physics,
    bool isDynamic,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
)
{
    this->physics = physics;
    this->isDynamic = isDynamic;
}

void Physics::PhysicsObject::PhysicsUpdate()
{
}

void Physics::PhysicsObject::syncTransformation(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
{
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