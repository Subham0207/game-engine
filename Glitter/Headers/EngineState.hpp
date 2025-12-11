#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include "Helpers/LimitedVector.hpp"
#include "UIState/UIState.hpp"
#include "Level/Level.hpp"
#include <filesystem>
#include <Controls/PlayerController.hpp>
#include <PhysicsSystem.hpp>
#include <EngineRegistry.hpp>
#include <LuaEngine/LuaEngine.hpp>
#include <Controls/statemachine.hpp>
namespace fs = std::filesystem;

namespace AI
{
    class AI;
}

class EngineState{
public:
    EngineState();
    static EngineState* state;
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;

    glm::vec3 rayEnd;

    bool isWorldSpace = true;

    std::string engineInstalledDirctory = fs::current_path().string();

    std::string currentActiveProjectDirectory = "";

    Engine::Registry* engineRegistry = new Engine::Registry();

    void printMat4(glm::mat4 mat)
    {
        std::cout << "mat[0]: " << mat[0][0] <<" "<< mat[0][1] <<" "<< mat[0][2] << " " << mat[0][3] << std::endl;
        std::cout << "mat[1]: " << mat[1][0] <<" "<< mat[1][1] <<" "<< mat[1][2] << " " << mat[1][3] << std::endl;
        std::cout << "mat[2]: " << mat[2][0] <<" "<< mat[2][1] <<" "<< mat[2][2] << " " << mat[2][3] << std::endl;
        std::cout << "mat[3]: " << mat[3][0] <<" "<< mat[3][1] <<" "<< mat[3][2] << " " << mat[3][3] << std::endl;
    }

    LimitedVector<std::string> errorStack = LimitedVector<std::string>(1000);
    LimitedVector<std::string> warningStack = LimitedVector<std::string>(1000);
    LimitedVector<std::string> successStack = LimitedVector<std::string>(1000);

    Level *activeLevel = new Level();//Init an empty level so this compiles

    int activeCameraIndex = 0;

    bool isPlay = false;

    int activePlayerControllerId = 0;
    std::vector<Controls::PlayerController*> playerControllers = std::vector<Controls::PlayerController*>();
    std::vector<AI::AI*> ais;
    std::vector<Controls::StateMachine*> statemachines = std::vector<Controls::StateMachine*>();

    ProjectAsset::UIState uiState = ProjectAsset::UIState();

    PhysicsSystemWrapper* physics = new PhysicsSystemWrapper();

    LuaEngine* luaEngine = new LuaEngine();
};

ProjectAsset::UIState& getUIState();
Level& getActiveLevel();
PhysicsSystemWrapper& getPhysicsSystem();
LuaEngine& getLuaEngine();
std::map<std::string, std::string> getEngineRegistryFilesMap();
