#pragma once
#include <Controls/statemachine.hpp>
#include<GenericFactory.hpp>

class WarriorStatemachine: public Controls::StateMachine
{
    REGISTER_BODY(WarriorStatemachine)

    public:
    WarriorStatemachine();
    
    virtual void onStart() override;

    virtual void onDestroy() override;

    virtual void onTick() override;

    BlendSpace2D* getLocomotionBlend(){return locomotionBlendspace;}
    void setGrounded(bool grounded){this->grounded = grounded;}
    void setDodgeStart(bool dodgeStart){this->dodgeStart = dodgeStart;}

    private:
        BlendSpace2D* locomotionBlendspace;
        bool grounded;
        bool dodgeStart;
};