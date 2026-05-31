#include <gtest/gtest.h>

#include "Character/Character.hpp"
#include "EngineState.hpp"

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

