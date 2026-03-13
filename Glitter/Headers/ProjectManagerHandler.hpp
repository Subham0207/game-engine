//
// Created by subha on 25-12-2025.
//

#ifndef GLITTER_PROJECTMANAGERHANDLER_HPP
#define GLITTER_PROJECTMANAGERHANDLER_HPP
#include "Controls/ClientHandler.hpp"
#include <filesystem>

#include "UI/ProjectManager.hpp"
namespace fs = std::filesystem;

class ProjectManagerHandler
{
public:
    ProjectManagerHandler();
    int startProjectManager();
private:
    GLFWwindow* mWindow;
    fs::path mPath;

    ProjectManagerUI::ProjectManager* projectManagerUI;

    float deltaTime;
    float lastFrame;
};


#endif //GLITTER_PROJECTMANAGERHANDLER_HPP