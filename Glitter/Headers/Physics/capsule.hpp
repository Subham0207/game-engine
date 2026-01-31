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
        Capsule (): PhysicsObject() {};
        Capsule(
            PhysicsSystemWrapper* physics,
            float radius = 0.5f,
            float halfHeight = 1.0f,
            bool isDynamic = false,
            bool shouldAddToLevel = false,
            glm::vec3 position = glm::vec3(0.0f),
            glm::quat rotation = glm::quat(),
            glm::vec3 scale = glm::vec3(1.0f)
        );
        ~Capsule()
        {
        }
        float mRadius;
        float mHalfHeight;

        void tick();
        std::shared_ptr<CapsuleColliderModel> capsule;
        void reInit(float radius, float halfheight);
        void syncTransformation() override;
        void addCustomModel(std::string modelPath) override;
        void moveBody(
            float deltaTime,
            glm::vec3 moveOffset,
            glm::quat rotationOffset,
            bool& want_jump,
            float walkSpeed,
            float jumpSpeed = 6.0f
        );
        void PhysicsUpdate() override;
        void Capsule::CreateCharacterVirtualPhysics(JPH::PhysicsSystem *system,
            const JPH::RVec3 &spawn, float halfheight = 0.8f, float radius = 0.3f);

        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;

        JPH::Ref<JPH::CharacterVirtualSettings> set;
        MyContactListener *listener;
        JPH::CharacterVirtual *character;

        glm::mat4 getWorldTransformation() const;
        glm::vec3 getWorldPosition();
        glm::quat getWorldRotation();
        glm::vec3 getWorldScale();

        void setWorldPosition(glm::vec3 position);
        void setWorldRotation(glm::quat rotation);

        // std::unique_ptr<JPH::TempAllocator> m_temp;

        bool grounded      = false;
        bool landed        = false;
        JPH::Vec3 ground_normal = JPH::Vec3::sAxisY();

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & mRadius;
            ar & mHalfHeight;
        }
    };
}