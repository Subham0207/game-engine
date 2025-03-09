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

    // blendFactor = getUIState().blendFactor;

    PlayAnimation(AnimationState::Walking);
}

void Controls::AnimationStateMachine::PlayAnimation(AnimationState state) {}