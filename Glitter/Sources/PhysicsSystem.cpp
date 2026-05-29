#include "PhysicsSystem.hpp"
#include <Physics/OpenGLJoltDebugRenderer.hpp>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>

#include <utility>

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

    debugRenderer = std::make_unique<Physics::OpenGLJoltDebugRenderer>();
}

void PhysicsSystemWrapper::Shutdown() {
    debugRenderer.reset();
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

JPH::BodyID PhysicsSystemWrapper::AddBox(const JPH::Vec3& pos, const JPH::Quat& rot, const JPH::Vec3& halfExtents, JPH::EMotionType motionType) {
    JPH::BodyCreationSettings settings(
        new JPH::BoxShape(halfExtents),
        pos,
        rot,
        motionType,
        0
    );

    JPH::BodyInterface& bi = physicsSystem.GetBodyInterface();
    const JPH::EActivation activation = motionType == JPH::EMotionType::Static
        ? JPH::EActivation::DontActivate
        : JPH::EActivation::Activate;
    JPH::BodyID id = bi.CreateAndAddBody(settings, activation);
    return id;
}

JPH::BodyID PhysicsSystemWrapper::AddSphere(const JPH::Vec3& pos, float radius, JPH::EMotionType motionType) {
    JPH::BodyCreationSettings settings(
        new JPH::SphereShape(radius),
        pos,
        JPH::Quat::sIdentity(),
        motionType,
        0
    );

    JPH::BodyInterface& bi = physicsSystem.GetBodyInterface();
    const JPH::EActivation activation = motionType == JPH::EMotionType::Static
        ? JPH::EActivation::DontActivate
        : JPH::EActivation::Activate;
    return bi.CreateAndAddBody(settings, activation);
}

JPH::BodyID PhysicsSystemWrapper::AddCapsule(const JPH::Vec3 &pos, const JPH::Quat &rot, float halfHeight, float radius, JPH::EMotionType motionType)
{
    JPH::BodyCreationSettings settings(
        new JPH::CapsuleShape(halfHeight, radius),
        pos,
        rot,
        motionType,
        0
    );
    const JPH::EActivation activation = motionType == JPH::EMotionType::Static
        ? JPH::EActivation::DontActivate
        : JPH::EActivation::Activate;
    return physicsSystem.GetBodyInterface().CreateAndAddBody(settings, activation);
}

JPH::BodyID PhysicsSystemWrapper::AddStaticMesh(
    const JPH::Vec3& pos,
    const JPH::Quat& rot,
    const JPH::VertexList& vertices,
    const JPH::IndexedTriangleList& triangles)
{
    if (vertices.empty() || triangles.empty())
    {
        return JPH::BodyID();
    }

    JPH::VertexList meshVertices = vertices;
    JPH::IndexedTriangleList meshTriangles = triangles;
    JPH::MeshShapeSettings meshSettings(std::move(meshVertices), std::move(meshTriangles));
    JPH::ShapeSettings::ShapeResult shapeResult = meshSettings.Create();
    if (shapeResult.HasError())
    {
        return JPH::BodyID();
    }

    JPH::BodyCreationSettings settings(
        shapeResult.Get(),
        pos,
        rot,
        JPH::EMotionType::Static,
        0
    );

    return physicsSystem.GetBodyInterface().CreateAndAddBody(settings, JPH::EActivation::DontActivate);
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

void PhysicsSystemWrapper::DrawDebugBodies(const glm::mat4& viewProjection, const glm::vec3& cameraPosition)
{
    if (!debugRenderer)
    {
        return;
    }

    debugRenderer->beginFrame();
    debugRenderer->SetCameraPos(JPH::RVec3(cameraPosition.x, cameraPosition.y, cameraPosition.z));

    JPH::BodyManager::DrawSettings drawSettings;
    drawSettings.mDrawShape = true;
    drawSettings.mDrawShapeWireframe = true;
    drawSettings.mDrawBoundingBox = false;

    physicsSystem.DrawBodies(drawSettings, debugRenderer.get());
    debugRenderer->render(viewProjection);
    debugRenderer->NextFrame();
}

