#pragma once
#include <Controls/statemachine.hpp>
#include <Helpers/Shared.hpp>
#include <EngineState.hpp>

Controls::AnimationStateMachine::AnimationStateMachine(PlayerController* playerController,  Animator* animator)
{
    this->playerController = playerController;
    this->animator = animator;
    currentState = AnimationState::Idle;
    previousState = AnimationState::Idle;
}

void Controls::AnimationStateMachine::Update() {
    if(playerController->isRunning)
        currentState = AnimationState::Running;
    else if(playerController->isWalking)
        currentState = AnimationState::Walking;
    else if(playerController->isJumping)
        currentState = AnimationState::Jumping;
    else
        currentState = AnimationState::Idle;

    if (currentState != previousState) {
        PlayAnimation(currentState);
        previousState = currentState;
    }
}

void Controls::AnimationStateMachine::PlayAnimation(AnimationState state) {
    switch (state) {
        case AnimationState::Running:
            animator->PlayAnimation(getUIState().animations[2]);
            break;
        case AnimationState::Walking:
            animator->PlayAnimation(getUIState().animations[1]);
            break;
        case AnimationState::Jumping:
            animator->PlayAnimation(getUIState().animations[3]);
            break;
        default:
            animator->PlayAnimation(getUIState().animations[0]);
    }
}