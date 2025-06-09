#include "PhysicsSystem.hpp"
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

PhysicsSystemWrapper::PhysicsSystemWrapper()
    : tempAllocator(nullptr), jobSystem(nullptr)
{
}

PhysicsSystemWrapper::~PhysicsSystemWrapper() {
    Shutdown();
}

void PhysicsSystemWrapper::Init() {
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
    jobSystem =  new JPH::JobSystemThreadPool(
        2048,
        8,
        std::thread::hardware_concurrency() - 1
    );

    // Minimal dummy filters for now

    static BroadPhaseLayerInterfaceImpl broadPhaseLayerInterface;

    static JPH::ObjectVsBroadPhaseLayerFilter objectVsBroadPhaseLayerFilter;
    static JPH::ObjectLayerPairFilter objectLayerPairFilter;

    physicsSystem.Init(
        1024, 0, 1024, 1024,
        broadPhaseLayerInterface,
        objectVsBroadPhaseLayerFilter,
        objectLayerPairFilter
    );
}

void PhysicsSystemWrapper::Shutdown() {
    JPH::UnregisterTypes();
    delete jobSystem;
    delete tempAllocator;
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

void PhysicsSystemWrapper::Update(float deltaTime) {
    if (!jobSystem || !tempAllocator) {
        throw std::runtime_error("PhysicsSystemWrapper not initialized!");
    }
    physicsSystem.Update(deltaTime, 1, tempAllocator, jobSystem);
}

JPH::BodyID PhysicsSystemWrapper::AddBox(const JPH::Vec3& pos, const JPH::Quat& rot, const JPH::Vec3& halfExtents, bool dynamic) {
    JPH::BodyCreationSettings settings(
        new JPH::BoxShape(halfExtents),
        pos,
        rot,
        dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
        0
    );

    JPH::BodyInterface& bi = physicsSystem.GetBodyInterface();
    JPH::BodyID id = bi.CreateAndAddBody(settings, dynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
    return id;
}

JPH::BodyID PhysicsSystemWrapper::AddSphere(const JPH::Vec3& pos, float radius, bool dynamic) {
    JPH::BodyCreationSettings settings(
        new JPH::SphereShape(radius),
        pos,
        JPH::Quat::sIdentity(),
        dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
        0
    );

    JPH::BodyInterface& bi = physicsSystem.GetBodyInterface();
    return bi.CreateAndAddBody(settings, dynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
}

JPH::BodyID PhysicsSystemWrapper::AddCapsule(const JPH::Vec3 &pos, const JPH::Quat &rot, float halfHeight, float radius, bool dynamic)
{
    JPH::BodyCreationSettings settings(
        new JPH::CapsuleShape(halfHeight, radius),
        pos,
        rot,
        JPH::EMotionType::Kinematic,
        0
    );
    return physicsSystem.GetBodyInterface().CreateAndAddBody(settings, dynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
}

JPH::Vec3 PhysicsSystemWrapper::GetBodyPosition(JPH::BodyID id) const {
    return physicsSystem.GetBodyInterface().GetCenterOfMassPosition(id);
}

JPH::Quat PhysicsSystemWrapper::GetBodyRotation(JPH::BodyID id) const {
    return physicsSystem.GetBodyInterface().GetRotation(id);
}

JPH::BodyInterface &PhysicsSystemWrapper::GetPhysicsBodyInterface()
{
    // TODO: insert return statement here
    return physicsSystem.GetBodyInterface();
}

void PhysicsSystemWrapper::RemoveBody(JPH::BodyID bodyID)
{
    if (GetPhysicsBodyInterface().IsAdded(bodyID)) {
        physicsSystem.GetBodyInterface().RemoveBody(bodyID);
    
        // Then destroy it completely
        physicsSystem.GetBodyInterface().DestroyBody(bodyID);
    }
}
