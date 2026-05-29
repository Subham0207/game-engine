#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Physics
{
    class OpenGLJoltDebugRenderer;
}

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

    JPH::BodyID AddBox(const JPH::Vec3& pos, const JPH::Quat& rot, const JPH::Vec3& halfExtents, JPH::EMotionType motionType);
    JPH::BodyID AddSphere(const JPH::Vec3& pos, float radius, JPH::EMotionType motionType);
    JPH::BodyID AddCapsule(const JPH::Vec3& pos, const JPH::Quat& rot, float halfHeight, float radius, JPH::EMotionType motionType);
    JPH::BodyID AddStaticMesh(const JPH::Vec3& pos, const JPH::Quat& rot, const JPH::VertexList& vertices, const JPH::IndexedTriangleList& triangles);

    JPH::Vec3 GetBodyPosition(JPH::BodyID id) const;
    JPH::Quat GetBodyRotation(JPH::BodyID id) const;
    JPH::BodyInterface& GetPhysicsBodyInterface();
    void RemoveBody(JPH::BodyID bodyID);
    void DrawDebugBodies(const glm::mat4& viewProjection, const glm::vec3& cameraPosition);
    JPH::PhysicsSystem physicsSystem;
private:
    JPH::TempAllocatorImpl* tempAllocator;
    JPH::JobSystemThreadPool* jobSystem;
    std::unique_ptr<Physics::OpenGLJoltDebugRenderer> debugRenderer;
};
