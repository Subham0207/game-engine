#include "PhysicsSystem.hpp"
#include <Physics/OpenGLJoltDebugRenderer.hpp>
#include <EngineState.hpp>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/ContactListener.h>

#include <Event/EventQueue.hpp>
#include <Renderable/renderable.hpp>

#include <utility>

class PhysicsBodyContactListener final : public JPH::ContactListener
{
public:
    explicit PhysicsBodyContactListener(PhysicsSystemWrapper* owner)
        : owner(owner)
    {
    }

        JPH::ValidateResult OnContactValidate(
            const JPH::Body& inBody1,
            const JPH::Body& inBody2,
            JPH::RVec3Arg inBaseOffset,
            const JPH::CollideShapeResult& inCollisionResult) override
        {
            return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
        }

        void OnContactAdded(
            const JPH::Body& inBody1,
            const JPH::Body& inBody2,
            const JPH::ContactManifold& inManifold,
            JPH::ContactSettings& ioSettings) override
        {
            record(inBody1, inBody2, inManifold);
        }

        void OnContactPersisted(
            const JPH::Body& inBody1,
            const JPH::Body& inBody2,
            const JPH::ContactManifold& inManifold,
            JPH::ContactSettings& ioSettings) override
        {
            record(inBody1, inBody2, inManifold);
        }

private:
    void record(const JPH::Body& bodyA, const JPH::Body& bodyB, const JPH::ContactManifold& manifold) const
    {
        if (owner == nullptr)
        {
            return;
        }

        std::vector<glm::vec3> contactPoints;
        contactPoints.reserve(manifold.mRelativeContactPointsOn1.size() + manifold.mRelativeContactPointsOn2.size());
        const JPH::RVec3 worldBaseA = bodyA.GetCenterOfMassTransform().GetTranslation();
        for (const JPH::Vec3& localPoint : manifold.mRelativeContactPointsOn1)
        {
            const JPH::RVec3 worldPoint = worldBaseA + JPH::RVec3(localPoint);
            contactPoints.emplace_back(
                static_cast<float>(worldPoint.GetX()),
                static_cast<float>(worldPoint.GetY()),
                static_cast<float>(worldPoint.GetZ()));
        }

        owner->recordBodyCollision(bodyA.GetID(), bodyB.GetID(), contactPoints);

        if (bodyA.IsSensor() || bodyB.IsSensor())
        {
            owner->recordSensorOverlap(bodyA.GetID(), bodyB.GetID());
        }
    }

    PhysicsSystemWrapper* owner = nullptr;
};

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
    static ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
    static ObjectLayerPairFilterImpl objectLayerPairFilter;

    physicsSystem.Init(
        1024, 0, 1024, 1024,
        broadPhaseLayerInterface,
        objectVsBroadPhaseLayerFilter,
        objectLayerPairFilter
    );

    bodyContactListener = std::make_unique<PhysicsBodyContactListener>(this);
    physicsSystem.SetContactListener(bodyContactListener.get());

    debugRenderer = std::make_unique<Physics::OpenGLJoltDebugRenderer>();
}

void PhysicsSystemWrapper::Shutdown() {
    debugRenderer.reset();
    bodyContactListener.reset();
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
    finalizePhysicsEventsForFrame();
}

JPH::BodyID PhysicsSystemWrapper::AddBox(
    const JPH::Vec3& pos,
    const JPH::Quat& rot,
    const JPH::Vec3& halfExtents,
    JPH::EMotionType motionType,
    const JPH::ObjectLayer objectLayer,
    const bool isSensor) {
    JPH::BodyCreationSettings settings(
        new JPH::BoxShape(halfExtents),
        pos,
        rot,
        motionType,
        objectLayer
    );
    settings.mIsSensor = isSensor;

    JPH::BodyInterface& bi = physicsSystem.GetBodyInterface();
    const JPH::EActivation activation = motionType == JPH::EMotionType::Static
        ? JPH::EActivation::DontActivate
        : JPH::EActivation::Activate;
    JPH::BodyID id = bi.CreateAndAddBody(settings, activation);
    return id;
}

JPH::BodyID PhysicsSystemWrapper::AddSphere(
    const JPH::Vec3& pos,
    const float radius,
    const JPH::EMotionType motionType,
    const JPH::ObjectLayer objectLayer,
    const bool isSensor) {
    JPH::BodyCreationSettings settings(
        new JPH::SphereShape(radius),
        pos,
        JPH::Quat::sIdentity(),
        motionType,
        objectLayer
    );
    settings.mIsSensor = isSensor;

    JPH::BodyInterface& bi = physicsSystem.GetBodyInterface();
    const JPH::EActivation activation = motionType == JPH::EMotionType::Static
        ? JPH::EActivation::DontActivate
        : JPH::EActivation::Activate;
    return bi.CreateAndAddBody(settings, activation);
}

JPH::BodyID PhysicsSystemWrapper::AddCapsule(
    const JPH::Vec3 &pos,
    const JPH::Quat &rot,
    const float halfHeight,
    const float radius,
    const JPH::EMotionType motionType,
    const JPH::ObjectLayer objectLayer,
    const bool isSensor)
{
    JPH::BodyCreationSettings settings(
        new JPH::CapsuleShape(halfHeight, radius),
        pos,
        rot,
        motionType,
        objectLayer
    );
    settings.mIsSensor = isSensor;
    const JPH::EActivation activation = motionType == JPH::EMotionType::Static
        ? JPH::EActivation::DontActivate
        : JPH::EActivation::Activate;
    return physicsSystem.GetBodyInterface().CreateAndAddBody(settings, activation);
}

JPH::BodyID PhysicsSystemWrapper::AddStaticMesh(
    const JPH::Vec3& pos,
    const JPH::Quat& rot,
    const JPH::VertexList& vertices,
    const JPH::IndexedTriangleList& triangles,
    const JPH::ObjectLayer objectLayer,
    const bool isSensor)
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
        objectLayer
    );
    settings.mIsSensor = isSensor;

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
    UnregisterBodyOwner(bodyID);
    if (GetPhysicsBodyInterface().IsAdded(bodyID)) {
        physicsSystem.GetBodyInterface().RemoveBody(bodyID);
    
        // Then destroy it completely
        physicsSystem.GetBodyInterface().DestroyBody(bodyID);
    }
}

PhysicsSystemWrapper::BodyPairKey PhysicsSystemWrapper::makeBodyPairKey(const JPH::BodyID bodyA, const JPH::BodyID bodyB)
{
    const uint32_t a = bodyA.GetIndexAndSequenceNumber();
    const uint32_t b = bodyB.GetIndexAndSequenceNumber();
    if (a <= b)
    {
        return {a, b};
    }

    return {b, a};
}

PhysicsSystemWrapper::InstancePairKey PhysicsSystemWrapper::makeInstancePairKey(const std::string& instanceA, const std::string& instanceB)
{
    return {instanceA, instanceB};
}

void PhysicsSystemWrapper::RegisterBodyOwner(
    const JPH::BodyID bodyID,
    Renderable* renderable,
    const std::string& instanceId,
    const bool isSensor)
{
    if (instanceId.empty())
    {
        return;
    }

    bodyOwners[bodyID.GetIndexAndSequenceNumber()] = BodyOwnerEntry{renderable, instanceId, isSensor};
}

void PhysicsSystemWrapper::UnregisterBodyOwner(const JPH::BodyID bodyID)
{
    bodyOwners.erase(bodyID.GetIndexAndSequenceNumber());
}

const PhysicsSystemWrapper::BodyOwnerEntry* PhysicsSystemWrapper::findBodyOwner(const JPH::BodyID bodyID) const
{
    const auto found = bodyOwners.find(bodyID.GetIndexAndSequenceNumber());
    if (found == bodyOwners.end())
    {
        return nullptr;
    }

    return &found->second;
}

void PhysicsSystemWrapper::recordBodyCollision(
    const JPH::BodyID bodyA,
    const JPH::BodyID bodyB,
    const std::vector<glm::vec3>& contactPoints)
{
    const auto* ownerA = findBodyOwner(bodyA);
    const auto* ownerB = findBodyOwner(bodyB);
    if (ownerA == nullptr || ownerB == nullptr || ownerA->instanceId == ownerB->instanceId)
    {
        return;
    }

    const BodyPairKey bodyPair = makeBodyPairKey(bodyA, bodyB);
    auto& points = bodyCollisionPointsByPair[bodyPair];
    points.insert(points.end(), contactPoints.begin(), contactPoints.end());
}

void PhysicsSystemWrapper::recordSensorOverlap(const JPH::BodyID bodyA, const JPH::BodyID bodyB)
{
    const auto* ownerA = findBodyOwner(bodyA);
    const auto* ownerB = findBodyOwner(bodyB);
    if (ownerA == nullptr || ownerB == nullptr || ownerA->instanceId == ownerB->instanceId)
    {
        return;
    }

    if (ownerA->isSensor)
    {
        sensorPairsThisFrame.insert(makeInstancePairKey(ownerA->instanceId, ownerB->instanceId));
    }
    if (ownerB->isSensor)
    {
        sensorPairsThisFrame.insert(makeInstancePairKey(ownerB->instanceId, ownerA->instanceId));
    }
}

bool PhysicsSystemWrapper::isInstanceAlive(const std::string& instanceId) const
{
    if (EngineState::state == nullptr || EngineState::state->activeLevel == nullptr)
    {
        return false;
    }

    const auto& instanceMap = EngineState::state->activeLevel->instanceIdToSerializableMap;
    return instanceMap.find(instanceId) != instanceMap.end();
}

Renderable* PhysicsSystemWrapper::resolveRenderableByInstanceId(const std::string& instanceId) const
{
    if (EngineState::state == nullptr || EngineState::state->activeLevel == nullptr)
    {
        return nullptr;
    }

    const auto& instanceMap = EngineState::state->activeLevel->instanceIdToSerializableMap;
    const auto found = instanceMap.find(instanceId);
    if (found == instanceMap.end())
    {
        return nullptr;
    }

    return dynamic_cast<Renderable*>(found->second.get());
}

void PhysicsSystemWrapper::pruneDestroyedInstanceEntries()
{
    for (auto it = bodyOwners.begin(); it != bodyOwners.end();)
    {
        if (!isInstanceAlive(it->second.instanceId))
        {
            it = bodyOwners.erase(it);
            continue;
        }

        ++it;
    }

    for (auto it = sensorPairsPreviousFrame.begin(); it != sensorPairsPreviousFrame.end();)
    {
        if (!isInstanceAlive(it->first) || !isInstanceAlive(it->second))
        {
            it = sensorPairsPreviousFrame.erase(it);
            continue;
        }
        ++it;
    }

    for (auto it = currentSensorOverlapsByInstance.begin(); it != currentSensorOverlapsByInstance.end();)
    {
        if (!isInstanceAlive(it->first))
        {
            it = currentSensorOverlapsByInstance.erase(it);
            continue;
        }

        for (auto overlapIt = it->second.begin(); overlapIt != it->second.end();)
        {
            if (!isInstanceAlive(*overlapIt))
            {
                overlapIt = it->second.erase(overlapIt);
                continue;
            }
            ++overlapIt;
        }

        ++it;
    }
}

void PhysicsSystemWrapper::finalizePhysicsEventsForFrame()
{
    pruneDestroyedInstanceEntries();

    EventQueue* queue = nullptr;
    if (EngineState::state != nullptr && EngineState::state->activeLevel != nullptr)
    {
        queue = EngineState::state->activeLevel->eventQueue;
    }

    if (queue != nullptr)
    {
        for (const auto& [pairKey, points] : bodyCollisionPointsByPair)
        {
            const auto bodyAIt = bodyOwners.find(pairKey.first);
            const auto bodyBIt = bodyOwners.find(pairKey.second);
            if (bodyAIt == bodyOwners.end() || bodyBIt == bodyOwners.end())
            {
                continue;
            }

            const auto& bodyA = bodyAIt->second;
            const auto& bodyB = bodyBIt->second;
            if (bodyA.isSensor || bodyB.isSensor)
            {
                continue;
            }

            queue->push<BodiesCollidedEvent>(
                bodyA.renderable,
                bodyB.renderable,
                bodyA.instanceId,
                bodyB.instanceId,
                points);
        }
    }

    std::unordered_map<std::string, std::unordered_set<std::string>> nextSensorOverlaps;
    for (const auto& pair : sensorPairsThisFrame)
    {
        nextSensorOverlaps[pair.first].insert(pair.second);
    }

    if (queue != nullptr)
    {
        for (const auto& pair : sensorPairsThisFrame)
        {
            if (sensorPairsPreviousFrame.find(pair) != sensorPairsPreviousFrame.end())
            {
                continue;
            }

            queue->push<SensorsEnteredEvent>(
                resolveRenderableByInstanceId(pair.first),
                resolveRenderableByInstanceId(pair.second),
                pair.first,
                pair.second);
        }

        for (const auto& pair : sensorPairsPreviousFrame)
        {
            if (sensorPairsThisFrame.find(pair) != sensorPairsThisFrame.end())
            {
                continue;
            }

            queue->push<SensorsExitedEvent>(
                resolveRenderableByInstanceId(pair.first),
                resolveRenderableByInstanceId(pair.second),
                pair.first,
                pair.second);
        }
    }

    currentSensorOverlapsByInstance = std::move(nextSensorOverlaps);
    sensorPairsPreviousFrame = sensorPairsThisFrame;
    sensorPairsThisFrame.clear();
    bodyCollisionPointsByPair.clear();
}

std::vector<Renderable*> PhysicsSystemWrapper::GetOverlappingSensorsFor(const std::string& instanceId) const
{
    std::vector<Renderable*> result;
    const auto found = currentSensorOverlapsByInstance.find(instanceId);
    if (found == currentSensorOverlapsByInstance.end())
    {
        return result;
    }

    result.reserve(found->second.size());
    for (const auto& otherId : found->second)
    {
        if (Renderable* other = resolveRenderableByInstanceId(otherId))
        {
            result.push_back(other);
        }
    }

    return result;
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

