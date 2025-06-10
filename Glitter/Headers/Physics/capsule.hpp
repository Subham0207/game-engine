#pragma once
#include <PhysicsSystem.hpp>
#include <Physics/PhysicsObject.hpp>
#include <3DModel/capsulecolliderMesh.hpp>
#include <glm/glm.hpp>

namespace Physics
{
    class Capsule: public PhysicsObject {
    public:
        Capsule(
            PhysicsSystemWrapper* physics,
            bool isDynamic = false,
            bool shouldAddToLevel = false,
            glm::vec3 position = glm::vec3(0.0f),
            glm::quat rotation = glm::quat(),
            glm::vec3 scale = glm::vec3(1.0f)
        );
        float radius = 0.5f;
        float halfHeight = 1.0f;
        CapsuleColliderModel *capsule;
        void reInit(float radius, float halfheight);
        void syncTransformation() override;
        void addCustomModel(std::string modelPath) override;
        void movebody(float x, float y, float z, float deltaTime);

        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };
}