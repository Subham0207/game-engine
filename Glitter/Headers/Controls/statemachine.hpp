#include <3DModel/Animation/Animator.hpp>

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
        Animator* animator;

    public:
        AnimationStateMachine();
        
        void Update() {
            switch (currentState) {
                case AnimationState::Idle:
                    //When an input key is set change the state correctly
                    currentState = AnimationState::Walking;
                    break;
                case AnimationState::Walking:
                    currentState = AnimationState::Idle;
                    break;
            }

            PlayAnimation(currentState);
        }

        void PlayAnimation(AnimationState state) {
            switch (state) {
                case AnimationState::Idle:
                    // Play the correct animation
                    break;
                case AnimationState::Walking:
                    break;
            }
        }
    };

}