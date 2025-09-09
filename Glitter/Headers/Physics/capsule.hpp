#pragma once
#include <PhysicsSystem.hpp>
#include <Physics/PhysicsObject.hpp>
#include <3DModel/capsulecolliderMesh.hpp>
#include <glm/glm.hpp>
#include <Physics/MyContactListener.hpp>
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
        void movebody(float x, float y, float z, float deltaTime, glm::vec3 characterCurrentPos, glm::quat glmYaw, bool& want_jump);
        void PhysicsUpdate() override;
        void Capsule::CreateCharacterVirtualPhysics(JPH::PhysicsSystem *system,
            const JPH::RVec3 &spawn, float halfheight = 0.8f, float radius = 0.3f);

        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;

        JPH::Ref<JPH::CharacterVirtualSettings> set;
        MyContactListener *listener;
        JPH::CharacterVirtual *character;

        // std::unique_ptr<JPH::TempAllocator> m_temp;

        bool grounded      = false;
        bool landed        = false;
        JPH::Vec3 ground_normal = JPH::Vec3::sAxisY();
    };
}