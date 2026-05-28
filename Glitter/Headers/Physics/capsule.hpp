#pragma once
#include <PhysicsSystem.hpp>
#include <Physics/PhysicsObject.hpp>
#include <glm/glm.hpp>
#include <Physics/MyContactListener.hpp>
#include <boost/serialization/access.hpp>
#include <cstdint>
#include <vector>
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
        ~Capsule();
        float mRadius;
        float mHalfHeight;

        void tick();
        void reInit(float radius, float halfheight);
        void syncTransformation();
        void moveBody(
            float deltaTime,
            glm::vec3 moveOffset,
            glm::quat rotationOffset,
            bool& want_jump,
            float movementSpeed,
            float jumpSpeed = 6.0f
        );
        void PhysicsUpdate() override;
        void Capsule::CreateCharacterVirtualPhysics(JPH::PhysicsSystem *system,
            const JPH::RVec3 &spawn, float halfheight = 0.8f, float radius = 0.3f);

        [[nodiscard]] bool hasCharacterCollision() const;
        [[nodiscard]] std::vector<uint32_t> getCollidingCharacterIds() const;
        [[nodiscard]] uint32_t getCharacterId() const;

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