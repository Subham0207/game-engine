#pragma once
#include <filesystem>
#include <Controls/statemachine.hpp>
namespace fs = std::filesystem;
class Character;
namespace AI
{
    class AI;
}

const int MAX_PROJECTS = 5;

static void write_text(const fs::path& p, const std::string& text);

int openAProject(const std::string& currentDir);

void update_recent_projects_list(const fs::path& projects_file_path, const fs::path& new_project_path);

Controls::StateMachine* setupStateMachine(fs::path projectAssetDirectory);

int create_new_project(const std::string& currentDir, const std::string& projectName);

void create_cmake_game_project(const std::string& projectDir, const std::string& projectName);

Character* addPlayableCharacter(std::filesystem::path root, std::filesystem::path projectAssetDirectory);

AI::AI* addAICharacter(std::filesystem::path root, Character* aiCharacter);

void GenerateLuaLSConfig(const fs::path& projectRoot);