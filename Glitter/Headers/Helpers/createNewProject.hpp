#pragma once
#include <filesystem>
#include <Controls/statemachine.hpp>
namespace fs = std::filesystem;

const int MAX_PROJECTS = 5;

static void write_text(const fs::path& p, const std::string& text);

int openAProject(const std::string& currentDir);

void update_recent_projects_list(const fs::path& projects_file_path, const fs::path& new_project_path);

Controls::StateMachine* setupStateMachine(fs::path projectAssetDirectory);

int create_new_project(const std::string& currentDir, const std::string& projectName);