#pragma once
#include<Character/Character.hpp>
#include<GenericFactory.hpp>
#include<Camera/ThirdPersonCameraSpringArm.hpp>
#include<WarriorStatemachine.hpp>

class WarriorCharacter: public Character
{
    REGISTER_BODY(WarriorCharacter)

    public:
        virtual void onStart() override;

        virtual void onDestroy() override;

        virtual void onTick() override;
    private:
        ThirdPersonCameraSpringArm springArm;
        InputHandler* inputHandler;

        bool grounded = false;
        bool isJumping = false;
        bool dodgeStart = false;
        bool isAiming = false;

        //These variables are used to play the blendspace
        double currentMovementSpeed = 0.0f;
        double currentMovementDirection = 0.0f;

        double targetSpeed = 0.0f;
        double targetDirection = 0.0f;
        //---------------------

        //These are used to move the character
        float inputXWorld = 0.0f;
        float inputZWorld = 0.0f;
        float interpolationSpeed = 0.1f;
        //--------------------------


        //spring arm related variable
        float lastPlayerYaw = 0.0f;
        //-----

        void handleInputs();
        void setMovement(glm::vec3 dir);
        void handleSpringArmAndCamera();

        std::shared_ptr<WarriorStatemachine> getDerivedStateMachine()
        {
            return std::static_pointer_cast<WarriorStatemachine>(this->animStateMachine);
        }
};