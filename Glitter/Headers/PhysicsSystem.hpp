#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Physics/PhysicsLayerRegistry.hpp"

namespace Physics
{
    class OpenGLJoltDebugRenderer;
}

class Renderable;
class PhysicsBodyContactListener;
class EventQueue;

class BroadPhaseLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface {
    public:
        virtual uint32_t GetNumBroadPhaseLayers() const override { return 1; }
        virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer) const override { return JPH::BroadPhaseLayer(0); }
        #if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
            return "Default";
        }
        #endif
};

class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer, JPH::BroadPhaseLayer) const override { return true; }
};

class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
    {
        return Physics::PhysicsLayerRulebookRegistry::instance().shouldCollide(inObject1, inObject2);
    }
};

class PhysicsSystemWrapper {
public:
    PhysicsSystemWrapper();
    ~PhysicsSystemWrapper();

    void Init();
    void Shutdown();
    void Update(float deltaTime);

    JPH::BodyID AddBody(const JPH::BodyCreationSettings& settings);
    JPH::BodyID AddBox(const JPH::Vec3& pos, const JPH::Quat& rot, const JPH::Vec3& halfExtents, JPH::EMotionType motionType, JPH::ObjectLayer objectLayer, bool isSensor);
    JPH::BodyID AddSphere(const JPH::Vec3& pos, float radius, JPH::EMotionType motionType, JPH::ObjectLayer objectLayer, bool isSensor);
    JPH::BodyID AddCapsule(const JPH::Vec3& pos, const JPH::Quat& rot, float halfHeight, float radius, JPH::EMotionType motionType, JPH::ObjectLayer objectLayer, bool isSensor);
    JPH::BodyID AddStaticMesh(const JPH::Vec3& pos, const JPH::Quat& rot, const JPH::VertexList& vertices, const JPH::IndexedTriangleList& triangles, JPH::ObjectLayer objectLayer, bool isSensor);

    JPH::Vec3 GetBodyPosition(JPH::BodyID id) const;
    JPH::Quat GetBodyRotation(JPH::BodyID id) const;
    JPH::BodyInterface& GetPhysicsBodyInterface();
    void RemoveBody(JPH::BodyID bodyID);
    void DrawDebugBodies(const glm::mat4& viewProjection, const glm::vec3& cameraPosition);

    void RegisterBodyOwner(JPH::BodyID bodyID, Renderable* renderable, const std::string& instanceId, bool isSensor);
    void UnregisterBodyOwner(JPH::BodyID bodyID);
    void setEventQueue(EventQueue* queue);
    [[nodiscard]] std::vector<Renderable*> GetOverlappingSensorsFor(const std::string& instanceId) const;

    struct BodyOwnerEntry
    {
        Renderable* renderable = nullptr;
        std::string instanceId;
        bool isSensor = false;
    };

    [[nodiscard]] const BodyOwnerEntry* findBodyOwner(JPH::BodyID bodyID) const;

    void recordBodyCollision(
        JPH::BodyID bodyA,
        JPH::BodyID bodyB,
        const std::vector<glm::vec3>& contactPoints);
    void recordSensorOverlap(JPH::BodyID bodyA, JPH::BodyID bodyB);
    void finalizePhysicsEventsForFrame();

    JPH::PhysicsSystem physicsSystem;
private:
    struct BodyPairKey
    {
        uint32_t first = 0;
        uint32_t second = 0;

        bool operator==(const BodyPairKey& rhs) const
        {
            return first == rhs.first && second == rhs.second;
        }
    };

    struct BodyPairKeyHasher
    {
        size_t operator()(const BodyPairKey& key) const noexcept
        {
            return (static_cast<size_t>(key.first) << 32u) ^ static_cast<size_t>(key.second);
        }
    };

    struct InstancePairKey
    {
        std::string first;
        std::string second;

        bool operator==(const InstancePairKey& rhs) const
        {
            return first == rhs.first && second == rhs.second;
        }
    };

    struct InstancePairKeyHasher
    {
        size_t operator()(const InstancePairKey& key) const noexcept
        {
            return std::hash<std::string>{}(key.first) ^ (std::hash<std::string>{}(key.second) << 1u);
        }
    };

    static BodyPairKey makeBodyPairKey(JPH::BodyID bodyA, JPH::BodyID bodyB);
    static InstancePairKey makeInstancePairKey(const std::string& instanceA, const std::string& instanceB);
    void pruneDestroyedInstanceEntries();
    [[nodiscard]] bool isInstanceAlive(const std::string& instanceId) const;
    [[nodiscard]] Renderable* resolveRenderableByInstanceId(const std::string& instanceId) const;

    JPH::TempAllocatorImpl* tempAllocator;
    JPH::JobSystemThreadPool* jobSystem;
    std::unique_ptr<Physics::OpenGLJoltDebugRenderer> debugRenderer;
    std::unique_ptr<PhysicsBodyContactListener> bodyContactListener;

    std::unordered_map<uint32_t, BodyOwnerEntry> bodyOwners;
    std::unordered_map<BodyPairKey, std::vector<glm::vec3>, BodyPairKeyHasher> bodyCollisionPointsByPair;
    std::unordered_set<InstancePairKey, InstancePairKeyHasher> sensorPairsThisFrame;
    std::unordered_set<InstancePairKey, InstancePairKeyHasher> sensorPairsPreviousFrame;
    std::unordered_map<std::string, std::unordered_set<std::string>> currentSensorOverlapsByInstance;
    EventQueue* eventQueue = nullptr;
};
