#pragma once
#include <3DModel/Animation/Animator.hpp>
#include <Controls/PlayerController.hpp>

namespace Controls
{
    enum class AnimationState {
        Idle,
        Walking,
        Running,
        Jumping
    };

    class AnimationStateMachine {
    private:
        AnimationState currentState;
        AnimationState previousState;
        Animator* animator;
        PlayerController* playerController;

    public:
        AnimationStateMachine(PlayerController* playerController, Animator* animator);
        
        void Update();
        void PlayAnimation(AnimationState state);
    };

}