#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "3DModel/model.hpp"
#include "EngineState.hpp"
#include "Event/Event.hpp"
#include "Event/EventQueue.hpp"
#include "Level/Level.hpp"
#include "PhysicsSystem.hpp"

namespace
{
    class PhysicsSystemEventsFixture : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            previousState = EngineState::state;
            EngineState::state = &engineState;
            engineState.activeLevel = &level;
            physicsSystem = new PhysicsSystemWrapper();
            engineState.physics = physicsSystem;
            physicsSystem->setEventQueue(&queue);
        }

        void TearDown() override
        {
            if (physicsSystem != nullptr)
            {
                physicsSystem->setEventQueue(nullptr);
            }
            EngineState::state = previousState;
        }

        std::shared_ptr<Model> makeRegisteredModel(const std::string& instanceId, const std::vector<std::string>& tags)
        {
            auto model = std::make_shared<Model>();
            model->setInstanceId(instanceId);
            model->setGameplayTags(tags);
            level.instanceIdToSerializableMap[instanceId] = model;
            return model;
        }

        EngineState engineState;
        Level level;
        PhysicsSystemWrapper* physicsSystem = nullptr;
        EventQueue queue;
        EngineState* previousState = nullptr;
    };
}

TEST_F(PhysicsSystemEventsFixture, shouldEmitBodiesCollidedEventUsingInjectedQueueReference)
{
    auto bodyA = makeRegisteredModel("body_a", {"Character", "Player"});
    auto bodyB = makeRegisteredModel("body_b", {"Model", "Damageable"});

    // Ensure this test verifies PhysicsSystem's queue member, not Level::eventQueue lookup.
    level.eventQueue = nullptr;

    const JPH::BodyID bodyAId(1);
    const JPH::BodyID bodyBId(2);
    physicsSystem->RegisterBodyOwner(bodyAId, bodyA.get(), bodyA->getInstanceId(), false);
    physicsSystem->RegisterBodyOwner(bodyBId, bodyB.get(), bodyB->getInstanceId(), false);

    physicsSystem->recordBodyCollision(bodyAId, bodyBId, {glm::vec3(1.0f, 2.0f, 3.0f)});
    physicsSystem->finalizePhysicsEventsForFrame();

    int bodiesCollidedCount = 0;
    queue.drain([&](Event& event)
    {
        if (event.type() != EventType::BodiesCollided)
        {
            return;
        }

        ++bodiesCollidedCount;
        const auto& collision = static_cast<BodiesCollidedEvent&>(event);
        EXPECT_EQ(collision.bodyAInstanceId, bodyA->getInstanceId());
        EXPECT_EQ(collision.bodyBInstanceId, bodyB->getInstanceId());
        ASSERT_EQ(collision.contactPoints.size(), 1u);

        EXPECT_TRUE(collision.bodyA != nullptr);
        EXPECT_TRUE(collision.bodyB != nullptr);
        EXPECT_TRUE(collision.bodyA->GetGameplayTags().find("Character") != collision.bodyA->GetGameplayTags().end());
        EXPECT_TRUE(collision.bodyB->GetGameplayTags().find("Model") != collision.bodyB->GetGameplayTags().end());
    });

    EXPECT_EQ(bodiesCollidedCount, 1);
}

TEST_F(PhysicsSystemEventsFixture, shouldEmitSensorEnterExitEventsAndTrackCurrentOverlaps)
{
    auto sensorOwner = makeRegisteredModel("sensor_owner", {"Sensor"});
    auto otherBody = makeRegisteredModel("other_body", {"Target"});

    const JPH::BodyID sensorBodyId(3);
    const JPH::BodyID otherBodyId(4);
    physicsSystem->RegisterBodyOwner(sensorBodyId, sensorOwner.get(), sensorOwner->getInstanceId(), true);
    physicsSystem->RegisterBodyOwner(otherBodyId, otherBody.get(), otherBody->getInstanceId(), false);

    physicsSystem->recordSensorOverlap(sensorBodyId, otherBodyId);
    physicsSystem->finalizePhysicsEventsForFrame();

    int enteredCount = 0;
    int exitedCount = 0;
    queue.drain([&](Event& event)
    {
        if (event.type() == EventType::SensorsEntered)
        {
            ++enteredCount;
            const auto& entered = static_cast<SensorsEnteredEvent&>(event);
            EXPECT_EQ(entered.sensorOwnerInstanceId, sensorOwner->getInstanceId());
            EXPECT_EQ(entered.otherBodyInstanceId, otherBody->getInstanceId());
        }
        else if (event.type() == EventType::SensorsExited)
        {
            ++exitedCount;
        }
    });

    EXPECT_EQ(enteredCount, 1);
    EXPECT_EQ(exitedCount, 0);

    const auto overlapsAfterEnter = physicsSystem->GetOverlappingSensorsFor(sensorOwner->getInstanceId());
    ASSERT_EQ(overlapsAfterEnter.size(), 1u);
    EXPECT_EQ(overlapsAfterEnter[0], otherBody.get());

    // No overlaps recorded this frame => previous overlap should emit an exit event.
    physicsSystem->finalizePhysicsEventsForFrame();

    enteredCount = 0;
    exitedCount = 0;
    queue.drain([&](Event& event)
    {
        if (event.type() == EventType::SensorsEntered)
        {
            ++enteredCount;
        }
        else if (event.type() == EventType::SensorsExited)
        {
            ++exitedCount;
            const auto& exited = static_cast<SensorsExitedEvent&>(event);
            EXPECT_EQ(exited.sensorOwnerInstanceId, sensorOwner->getInstanceId());
            EXPECT_EQ(exited.otherBodyInstanceId, otherBody->getInstanceId());
        }
    });

    EXPECT_EQ(enteredCount, 0);
    EXPECT_EQ(exitedCount, 1);
    EXPECT_TRUE(physicsSystem->GetOverlappingSensorsFor(sensorOwner->getInstanceId()).empty());
}


