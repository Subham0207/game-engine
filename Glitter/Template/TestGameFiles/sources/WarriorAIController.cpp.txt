#include <WarriorAIController.hpp>
#include <iostream>

REGISTER_TYPE(WarriorAIController, "WarriorAIController", ControllerFactory)

WarriorAIController::WarriorAIController(): AI() {
}

void WarriorAIController::setMovement(glm::vec3 dir) {
    if(glm::length(dir) > 0.00001f)
    {
        targetSpeed = 1.0f; // to always return walking forward blendpoint when not aiming.
        targetDirection = 0.0f;
    }
    else
    {
        targetSpeed = 0.0f;
        targetDirection = 0.0f;
    }

    moveDirectionWorld = dir;
};

void WarriorAIController::OnStart()
{
    AI::AI::OnStart();
}

void WarriorAIController::moveTo(glm::vec3 position)
{
    if(grounded && !pathGenerated)
    {
        calculatePath(
            glm::vec3{currentCharacterPosition.x, 0.0f, currentCharacterPosition.z},
            position
        );
        pathGenerated = true;
        currentPathIndex = 0;
    }
}

void WarriorAIController::OnTick(float deltaTime)
{
    
    AI::AI::OnTick(deltaTime);

    this->deltatime = deltaTime;

    if(!started) return;

    if(!grounded) return;

    auto path = getPath();

    //MoveToGeneratesAPath... Then on on Tick we are following that path.
    //.....................    
    
    //This if check if MoveTo to a point is completed.
    //This is correct because only then we want to move to the next patrol point.
    if(currentPathIndex == path.size())
    {
        auto patrolPoints = getVariables();

        // Not on the path yet. So, go to first point.
        if(currentPatrolPointIndex == -1)
        {
            std::cout << "Moving to first point" << std::endl;
            nextPatrolPointIndex = 0;
        }
        else 
        {
            std::cout << "Deciding direction on path..." << std::endl;
            if(currentPatrolPointIndex == 0)
            {
                std::cout << "..Moving forward on path" << std::endl;
                isMovingForwardOnPatrolPath = true;
            }
            if(currentPatrolPointIndex == patrolPoints.size() - 1)
            {
                std::cout << "..Moving backwards on path" << std::endl;
                isMovingForwardOnPatrolPath =  false;
            }

        }

        // Reached currentPatrolPointIndex
        if(currentPatrolPointIndex > -1)
        {
            if
            (
                glm::length(
                    glm::vec3{
                        patrolPoints[currentPatrolPointIndex].x,
                        currentCharacterPosition.y,
                        patrolPoints[currentPatrolPointIndex].z
                    } - currentCharacterPosition) < 1.0f
            )
            {
                std::cout << "Deciding where to go next..." << std::endl;
                nextPatrolPointIndex = isMovingForwardOnPatrolPath ? currentPatrolPointIndex + 1: currentPatrolPointIndex - 1;
                pathGenerated = false;
            }
        }
        
        std::cout << "Moving to next position " << nextPatrolPointIndex << std::endl;
        this->moveTo(patrolPoints[nextPatrolPointIndex]);
        currentPatrolPointIndex = nextPatrolPointIndex;
    }

    //.....................

    if(path.size() == 0 || currentPathIndex == path.size())
    {
        setMovement(glm::vec3(0.0f,0.0f,0.0f));
        return;
    }
    // Get player current position
    glm::vec3 pos = currentCharacterPosition;
    const float MAX_DT = 0.1f; // 100ms, ~10 FPS worst case
    if (deltaTime > MAX_DT) deltaTime = MAX_DT;

    elapsedTime += deltaTime;

    glm::vec3 target = path[currentPathIndex];
    glm::vec3 posXZ    = glm::vec3(pos.x,    0.0f, pos.z);
    glm::vec3 targetXZ = glm::vec3(target.x, 0.0f, target.z);
    auto speed = 1.0f;

    targetWorldDirection = targetXZ - posXZ;
    float targetDistance = glm::length(targetWorldDirection);

    if (targetDistance < arrivalRadius)
    {
        currentPathIndex++;
        setMovement(glm::vec3(0.0f));
        return;
    }

    glm::vec3 dir = targetWorldDirection / targetDistance; // normalized safely
    glm::vec3 movement = dir * speed;
    setMovement(movement);
}