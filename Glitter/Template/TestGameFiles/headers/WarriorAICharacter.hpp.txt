#pragma once 
#include<Character/Character.hpp>
#include<GenericFactory.hpp>
#include<WarriorAIController.hpp>
#include<WarriorStatemachine.hpp>

class WarriorAICharacter: public Character
{
    REGISTER_BODY(WarriorAICharacter)

    public:
        void onStart() override;

        void onDestroy() override;

        void onTick() override;

    private:
        std::shared_ptr<WarriorAIController> aiController;
        std::shared_ptr<WarriorStatemachine> sm;

        float interpolationSpeed = 0.1f;
        float lastPlayerYaw = 0.0f;
};