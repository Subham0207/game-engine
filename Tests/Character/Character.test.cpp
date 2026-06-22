#include <gtest/gtest.h>

#include "Character/Character.hpp"
#include "EngineState.hpp"

namespace
{
    class CharacterAnimationEventProbe final : public Character
    {
    public:
        void onStart() override
        {
            animationEventBus->subscribe<AnimEventType::RegionEnter>(
                [this](const std::string& regionName)
                {
                    if (regionName == "Attack")
                        receivedEnterEvent = true;
                });
        }

        void onTick() override
        {
            observedEventInTick = receivedEnterEvent;
        }

        bool receivedEnterEvent = false;
        bool observedEventInTick = false;
    };
}

TEST(CharacterGameplayTags, shouldQueryConfiguredTags)
{
    EngineState engineState;
    EngineState::state = &engineState;

    {
        Character character;
        character.model = nullptr;
        character.animator = nullptr;
        character.skeleton = nullptr;
        character.capsuleCollider = nullptr;
        character.camera = nullptr;

        character.setGameplayTags({"Player", "Damageable", "Hero"});

        EXPECT_TRUE(character.hasGameplayTag("Player"));
        EXPECT_TRUE(character.hasGameplayTag("Damageable"));
        EXPECT_FALSE(character.hasGameplayTag("Enemy"));
        EXPECT_EQ(character.GetGameplayTags().size(), 3u);
    }

    EngineState::state = nullptr;
}

TEST(CharacterAnimationEvents, shouldDispatchQueuedAnimationEventsBeforeDerivedOnTick)
{
    EngineState engineState;
    EngineState::state = &engineState;

    {
        CharacterAnimationEventProbe character;
        character.model = nullptr;
        character.animator = nullptr;
        character.skeleton = nullptr;
        character.capsuleCollider = nullptr;
        character.camera = nullptr;

        character.onStart();
        character.animationEventQueue->push<AnimEventType::RegionEnter>("Attack");
        character.dispatchPendingAnimationEvents();
        character.onTick();

        EXPECT_TRUE(character.receivedEnterEvent);
        EXPECT_TRUE(character.observedEventInTick);
    }

    EngineState::state = nullptr;
}
