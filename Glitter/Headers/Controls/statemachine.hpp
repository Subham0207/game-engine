#pragma once
#include <3DModel/Animation/Animation.hpp>
#include <3DModel/Animation/Animator.hpp>
#include <Controls/PlayerController.hpp>
#include <Controls/BlendSpace2D.hpp>
#include <vector>
#include <functional>

namespace Controls
{
    struct State;
    struct ToStateWhenCondition
    {
        State* state = NULL;
        std::function<bool()> condition;

        ToStateWhenCondition(State* state, std::function<bool()> condition);
    };

    struct State
    {
        std::string stateName;
        std::vector<ToStateWhenCondition>* toStateWhenCondition;
        Animation* animation;
        BlendSpace2D* blendspace;

        State(std::string stateName);
        void Play(Controls::PlayerController* playerController, Animator* animator);
    }; 

    class StateMachine {
        public:
            StateMachine();
            void tick(Controls::PlayerController* playerController, Animator* animator);
            void setActiveState(State* state);
        private:
            State* stateGraph;
            State* activeState;
    };

}