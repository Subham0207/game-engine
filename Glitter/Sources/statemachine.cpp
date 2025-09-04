#pragma once
#include <Controls/statemachine.hpp>
#include <Helpers/Shared.hpp>
#include <EngineState.hpp>

Controls::ToStateWhenCondition::ToStateWhenCondition(State* state, std::function<bool()> condition)
{
    this->state = state;
    this->condition = condition;
}

Controls::State::State(std::string stateName)
{
    this->stateName = stateName;
    toStateWhenCondition = new std::vector<ToStateWhenCondition>();
    animation = NULL;
    blendspace = NULL;
}

void Controls::State::Play(Controls::PlayerController* playerController, Animator* animator)
{
    if(animation)
    {
        animator->PlayAnimation(animation);
        return;
    }

    if(blendspace)
    {
        float xfactor = playerController->movementDirection;
        float yfactor = playerController->movementSpeed;
        
        auto blendSelection = blendspace->GetBlendSelection(glm::vec2(xfactor, yfactor));
        animator->PlayAnimationBlended(blendSelection);
    }
}

Controls::StateMachine::StateMachine()
{
    stateGraph = NULL;
    activeState = NULL;
};

void Controls::StateMachine::tick(Controls::PlayerController* playerController, Animator* animator)
{
    if(!activeState)
    return;

    activeState->Play(playerController, animator);  

    for (size_t i = 0; i < activeState->toStateWhenCondition->size(); i++)
    {
        if(activeState->toStateWhenCondition->at(i).condition)
        {
            activeState = activeState->toStateWhenCondition->at(i).state;
            break;
        }
    }
    
}

void Controls::StateMachine::setActiveState(State* state)
{
    this->activeState = state;
}