#pragma once
#include <GenericFactory.hpp>
#include <AI/AI.hpp>

class WarriorAICharacter;
class WarriorAIController: public AI::AI
{
    REGISTER_BODY(WarriorAIController)

    public:
        WarriorAIController();

        void OnStart() override;
        void OnTick(float deltaTime) override;
        void OnDestroy() override {};
        
        void setMovement(glm::vec3 dir);

        void setCurrentCharacterPosition(glm::vec3 pos)
        {
            this->currentCharacterPosition = pos;
        }

        glm::vec3 getTargetWorldDirection()
        {
            return this->moveDirectionWorld;
        }

        void setGrounded(bool g)
        {
            this->grounded = g;
        }

        void moveTo(glm::vec3 position);

    private:
        double targetSpeed = 0.0f;
        double targetDirection = 0.0f;

        glm::vec3 targetWorldDirection{0.0f};

        int currentPathIndex = 0;
        float arrivalRadius = 0.1f;
        float elapsedTime = 0.0f;

        glm::vec3 currentCharacterPosition{};

        bool grounded = false;
        bool pathGenerated = false;

        glm::vec3 moveDirectionWorld{};

        float deltatime = 0.0f;

        bool isMovingForwardOnPatrolPath = true;
        int currentPatrolPointIndex = -1;
        int nextPatrolPointIndex = -1;
};