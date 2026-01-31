#include <WarriorStatemachine.hpp>

REGISTER_TYPE(WarriorStatemachine, "WarriorStatemachine", StateMachineFactory)

WarriorStatemachine::WarriorStatemachine(): Controls::StateMachine("characterMovements")
{
    grounded = false;
    dodgeStart = false;
}

void WarriorStatemachine::onStart()
{
    //state machine 
    auto locomotionState = std::make_shared<Controls::State>("Locomotion");
    auto jumpState = std::make_shared<Controls::State>("Jump");
    auto dodgeRollState = std::make_shared<Controls::State>("DodgeRoll");

    locomotionState->toStateWhenCondition.push_back(
        Controls::ToStateWhenCondition(
        jumpState,
        [&]() -> bool {return !grounded;})
    );
    
    jumpState->toStateWhenCondition.push_back(
        Controls::ToStateWhenCondition(
        locomotionState,
        [&]() -> bool {return grounded;})
    );

    locomotionState->toStateWhenCondition.push_back(
        Controls::ToStateWhenCondition(
        dodgeRollState,
        [&]() -> bool {return dodgeStart;})
    );

    dodgeRollState->toStateWhenCondition.push_back(
        Controls::ToStateWhenCondition(
        locomotionState,
        [&]() -> bool {return !dodgeStart;})
    );

    this->setActiveState(locomotionState);

    locomotionBlendspace = new BlendSpace2D("character_blendspace");
    locomotionState->assignBlendspace(locomotionBlendspace);

    locomotionState->blendspace->AddBlendPoint(glm::vec2(0.0f, 0.0f), "2bd905e6-913b-480d-8e36-a4a40b486f08");
    locomotionState->blendspace->AddBlendPoint(glm::vec2(-1.0f, 0.0f), "a277a1d6-e9b5-4fa7-9710-109a557a6df5");
    locomotionState->blendspace->AddBlendPoint(glm::vec2(1.0f, 0.0f), "e6890dfd-bbaa-4abd-92bc-35e00da7a274");

    locomotionState->blendspace->AddBlendPoint(glm::vec2(0.0f, 1.0f), "31cbc593-ba3c-4731-a294-8b27a6cbd2c9");
    locomotionState->blendspace->AddBlendPoint(glm::vec2(-1.0f, 1.0f), "a1a1526f-04a8-45a6-9a67-efa1b5b9086e");
    locomotionState->blendspace->AddBlendPoint(glm::vec2(1.0f, 1.0f), "e6890dfd-bbaa-4abd-92bc-35e00da7a274");

    locomotionState->blendspace->AddBlendPoint(glm::vec2(0.0f, 2.0f), "e6890dfd-bbaa-4abd-92bc-35e00da7a274");
    locomotionState->blendspace->AddBlendPoint(glm::vec2(-1.0f, 2.0f), "a277a1d6-e9b5-4fa7-9710-109a557a6df5");
    locomotionState->blendspace->AddBlendPoint(glm::vec2(1.0f, 2.0f), "e6890dfd-bbaa-4abd-92bc-35e00da7a274");

    locomotionState->blendspace->AddBlendPoint(glm::vec2(-1.0f, -1.0f), "a1a1526f-04a8-45a6-9a67-efa1b5b9086e");
    locomotionState->blendspace->AddBlendPoint(glm::vec2(0.0f, -1.0f), "a1a1526f-04a8-45a6-9a67-efa1b5b9086e");
    locomotionState->blendspace->AddBlendPoint(glm::vec2(1.0f, -1.0f), "a1a1526f-04a8-45a6-9a67-efa1b5b9086e");

    jumpState->assignAnimation("7f7f71ad-9714-4083-88bb-1e242b47d010", false, []{});
    dodgeRollState->assignAnimation("a403d7b9-fc28-439a-8022-f5470d864fea", true, [&]{
        dodgeStart = false;
    });
}

void WarriorStatemachine::onTick()
{
}

void WarriorStatemachine::onDestroy()
{
}
