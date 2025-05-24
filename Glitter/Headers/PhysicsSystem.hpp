#pragma once

#define JPH_PROFILE_ENABLED
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>

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

class PhysicsSystemWrapper {
public:
    PhysicsSystemWrapper();
    ~PhysicsSystemWrapper();

    void Init();
    void Shutdown();
    void Update(float deltaTime);

    JPH::BodyID AddBox(const JPH::Vec3& pos, const JPH::Quat& rot, const JPH::Vec3& halfExtents, bool dynamic);
    JPH::BodyID AddSphere(const JPH::Vec3& pos, float radius, bool dynamic);
    JPH::BodyID AddCapsule(const JPH::Vec3& pos, const JPH::Quat& rot, float halfHeight, float radius, bool dynamic);

    JPH::Vec3 GetBodyPosition(JPH::BodyID id) const;
    JPH::Quat GetBodyRotation(JPH::BodyID id) const;
    JPH::BodyInterface& GetPhysicsBodyInterface();
    void RemoveBody(JPH::BodyID bodyID);
    bool isFirstPhysicsEnabledFrame = true;
private:
    JPH::TempAllocatorImpl* tempAllocator;
    JPH::JobSystemThreadPool* jobSystem;
    JPH::PhysicsSystem physicsSystem;
};
