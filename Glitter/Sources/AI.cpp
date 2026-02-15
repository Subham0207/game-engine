#include <AI/AI.hpp>
#include <Controls/PlayerController.hpp>
#include <EngineState.hpp>
#include <random>
#include <Character/Character.hpp>
#include <Modals/CameraType.hpp>

AI::AI::AI(std::shared_ptr<Character> character, std::string filename): Serializable()
{
    // this->playerController = character->controller;
    this->controlledCharacterInstanceId = character->getInstanceId();
    this->filename = filename;
    targetDirection = glm::vec3(0.0f,0.0f,0.0f);
    targetDirChoosen = false;
    elapsedTime = 0.0f;
    arrivalRadius = 0.1f; // keep it bigger than or equal to max_step = speed * dt;
    currentPathIndex = 0;

    EngineState::state->ais.push_back(this);
}
void AI::AI::OnStart()
{
}
void AI::AI::OnTick(float deltaTime)
{
 //check if a target direction is choosen.
 //--- If yes, then pass that to playercontroller.
 //----- wait say 2 sec and then set target direction choosen to false.
 //--- If no, then choose a direction.
    started = EngineState::state->isPlay;
    if(started)
    {
        this->OnStart();
    }
    else
    {
        return;
    }
    if(path.size() == 0) return;
    // if(currentPathIndex == path.size())
    // {
    //     // playerController->setMovement(glm::vec3(0.0f,0.0f,0.0f));
    //     return;
    // }
    // // glm::vec3 pos = playerController->characterPosition;
    // glm::vec3 pos = glm::vec3{};
    // const float MAX_DT = 0.1f; // 100ms, ~10 FPS worst case
    // if (deltaTime > MAX_DT) deltaTime = MAX_DT;
    //
    // elapsedTime += deltaTime;
    //
    // glm::vec3 target = path[currentPathIndex];
    // glm::vec3 posXZ    = glm::vec3(pos.x,    0.0f, pos.z);
    // glm::vec3 targetXZ = glm::vec3(target.x, 0.0f, target.z);
    // auto speed = 20.0f;
    //
    // targetDirection = targetXZ - posXZ;
    // float targetDistance = glm::length(targetDirection);
    //
    // if (targetDistance < arrivalRadius)
    // {
    //     currentPathIndex++;
    //     // playerController->setMovement(glm::vec3(0.0f));
    //     return;
    // }
    //
    // glm::vec3 dir = targetDirection / targetDistance; // normalized safely
    // glm::vec3 movement = dir * speed * deltaTime;
    // playerController->setMovement(movement);

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

void AI::AI::saveContent(fs::path contentFileLocation, std::ostream &os)
{
    fs::path dir = contentFileLocation.parent_path();
    if (dir.empty()) {
        // Set the directory to the current working directory
        dir = fs::current_path();
    }
    if (!fs::exists(dir)) {
        if (!fs::create_directories(dir)) {
            std::cerr << "Failed to create directories: " << dir << std::endl;
            return;
        }
    }
    std::ofstream ofs(contentFileLocation.string());
    boost::archive::text_oarchive oa(ofs);
    oa << *this;
    ofs.close();

    // getActiveLevel().AIs.push_back(this);
}

void AI::AI::loadContent(fs::path contentFileLocation, std::istream &is)
{
    std::ifstream ifs(contentFileLocation.string());
    boost::archive::text_iarchive ia(ifs);
    ia >> *this;
    //use controlledCharacterInstanceId re-assign playerController.
    // hook to character if present
    if (auto character = std::dynamic_pointer_cast<Character>(getActiveLevel().instanceIdToSerializableMap[controlledCharacterInstanceId]))
    {
        // playerController = character->controller;
    }

    EngineState::state->ais.push_back(this);

    //attach temp ui
    getUIState().ai = this;
}
