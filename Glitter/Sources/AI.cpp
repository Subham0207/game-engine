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
}
void AI::AI::onStart()
{
}
void AI::AI::Tick(float deltaTime)
{
 //check if a target direction is choosen.
 //--- If yes, then pass that to playercontroller.
 //----- wait say 2 sec and then set target direction choosen to false.
 //--- If no, then choose a direction.
    if(!EngineState::state->isPlay) return;

    elapsedTime += deltaTime;
    if(targetDirChoosen)
    {
        playerController->setMovement(targetDirection);
        if(elapsedTime > 2.0f)
        {
            targetDirChoosen = false;
            elapsedTime = 0.0f;
        }
    }
    else
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(-1.0, 1.0);
        float randomNum = dist(gen);
        targetDirection = glm::vec3(randomNum,0.0f,randomNum);
        targetDirChoosen = true;
    }
}
