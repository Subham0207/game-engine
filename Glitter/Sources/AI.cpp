#include <AI/AI.hpp>
#include <Controls/PlayerController.hpp>
#include <EngineState.hpp>
#include <random>

AI::AI::AI(Controls::PlayerController* playerController)
{
    this->playerController = playerController;
    targetDirection = glm::vec3(0.0f,0.0f,0.0f);
    targetDirChoosen = false;
    elapsedTime = 0.0f;
    arrivalRadius = 0.1f; // keep it bigger than or equal to max_step = speed * dt;
    currentPathIndex = 0;
}
void AI::AI::onStart()
{
}
void AI::AI::Tick(float deltaTime, glm::vec3 pos)
{
 //check if a target direction is choosen.
 //--- If yes, then pass that to playercontroller.
 //----- wait say 2 sec and then set target direction choosen to false.
 //--- If no, then choose a direction.
    if(!EngineState::state->isPlay) return;
    if(path.size() == 0) return;
    if(currentPathIndex == path.size())
    {
        playerController->setMovement(glm::vec3(0.0f,0.0f,0.0f));
        return;
    }
    const float MAX_DT = 0.1f; // 100ms, ~10 FPS worst case
    if (deltaTime > MAX_DT) deltaTime = MAX_DT;

    elapsedTime += deltaTime;

    glm::vec3 target = path[currentPathIndex];
    glm::vec3 posXZ    = glm::vec3(pos.x,    0.0f, pos.z);
    glm::vec3 targetXZ = glm::vec3(target.x, 0.0f, target.z);
    auto speed = 20.0f;

    targetDirection = targetXZ - posXZ;
    float targetDistance = glm::length(targetDirection);

    if (targetDistance < arrivalRadius)
    {
        currentPathIndex++;
        playerController->setMovement(glm::vec3(0.0f));
        return;
    }

    glm::vec3 dir = targetDirection / targetDistance; // normalized safely
    glm::vec3 movement = dir * speed * deltaTime;
    playerController->setMovement(movement);

    //randomly going around
    // if(targetDirChoosen)
    // {
    //     playerController->setMovement(targetDirection);
    //     if(elapsedTime > 2.0f)
    //     {
    //         targetDirChoosen = false;
    //         elapsedTime = 0.0f;
    //     }
    // }
    // else
    // {
    //     std::random_device rd;
    //     std::mt19937 gen(rd());
    //     std::uniform_real_distribution<double> dist(-1.0, 1.0);
    //     float randomNum = dist(gen);
    //     targetDirection = glm::vec3(randomNum,0.0f,randomNum);
    //     targetDirChoosen = true;
    // }
}

void AI::AI::setPath(std::vector<glm::vec3> path)
{
    this->path = path;
}

void AI::AI::calculatePath(glm::vec3 startingPos, glm::vec3 targetPos)
{
    currentPathIndex = 0.0f;
    path.clear();

    std::vector<float> startingLocFloat;
    startingLocFloat.push_back(startingPos[0]);
    startingLocFloat.push_back(startingPos[1]);
    startingLocFloat.push_back(startingPos[2]);

    std::vector<float> targetLocFloat;
    targetLocFloat.push_back(targetPos[0]);
    targetLocFloat.push_back(targetPos[1]);
    targetLocFloat.push_back(targetPos[2]);

    std::vector<float> outPath;
    getActiveLevel().FindPath(startingLocFloat.data(), targetLocFloat.data(),outPath);
    for (size_t i = 0; i < outPath.size(); i+=3)
    {
        auto node = glm::vec3(
            outPath[i + 0],
            outPath[i + 1],
            outPath[i + 2]
        );
        path.push_back(node);
    }
}
