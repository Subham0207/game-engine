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
    this->blendFactor = 0.0f;
}

void Controls::AnimationStateMachine::Update() {
    // if(playerController->isRunning)
    //     currentState = AnimationState::Running;
    // else if(playerController->isWalking)
    //     currentState = AnimationState::Walking;
    // else if(playerController->isJumping)
    //     currentState = AnimationState::Jumping;
    // else
    //     currentState = AnimationState::Idle;

    // if (currentState != previousState) {
    //     PlayAnimation(currentState);
    //     previousState = currentState;

    //     blendFactor = 0.0f;

    // }

    // if(blendFactor <= 1)
    // blendFactor+=0.001;

    blendFactor = getUIState().blendFactor;

    PlayAnimation(AnimationState::Walking);
}

void Controls::AnimationStateMachine::PlayAnimation(AnimationState state) {
    switch (state) {
        case AnimationState::Running: // transition from walking to running
            animator->PlayAnimationBlended(getUIState().animations[1], getUIState().animations[2], blendFactor);
            break;
        case AnimationState::Walking: // transition from idle as 0.0 to walking as 1.0f
            animator->PlayAnimationBlended(getUIState().animations[0], getUIState().animations[1], blendFactor);
            break;
        case AnimationState::Jumping:
            animator->PlayAnimationBlended(getUIState().animations[3], getUIState().animations[0], blendFactor);
            break;
        default:
            animator->PlayAnimationBlended(getUIState().animations[0], getUIState().animations[1], blendFactor);
    }
}