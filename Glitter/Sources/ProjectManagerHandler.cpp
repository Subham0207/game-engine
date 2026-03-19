//
// Created by subha on 25-12-2025.
//

#include "../Headers/ProjectManagerHandler.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdlib>
#include <windows.h>

#include "Helpers/shader.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <Helpers/Shared.hpp>

#include <UI/ProjectManager.hpp>

#include "Controls/ClientHandler.hpp"
#include "Helpers/GetExecutablePath.hpp"
#include <fstream>

ProjectManagerHandler::ProjectManagerHandler() : mWindow(nullptr)
{
    mPath = GetExecutablePath::getExecutableDir();
    deltaTime = 0.0;
    lastFrame = 0.0f;
    projectManagerUI = new ProjectManagerUI::ProjectManager(mPath.string(), mPath.string());
}

int ProjectManagerHandler::startProjectManager()
{
    // Project manager only needs this for legacy input plumbing.
    // Kept local to avoid global singleton state.
    auto clientHandler = std::make_unique<ClientHandler>();

    if (fs::exists(mPath / "user_prefs.json")) {
        std::ifstream infile(mPath / "user_prefs.json");
        std::string line;
        while (std::getline(infile, line)) {
            // Check if the line is not empty before adding.
            if (!line.empty()) {
                projectManagerUI->addToRecentProjects(fs::path(line));
            }
        }
        infile.close();
    }

    mWindow = Shared::initAWindow(true);

    Shared::initImguiBackend(mWindow);

    while (glfwWindowShouldClose(mWindow) == false)
    {

        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //////Code changes go here//////
        projectManagerUI->draw();
        ////////////////////////////////

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(mWindow);
        glfwPollEvents();

        // Now it's safe to leave the loop
        //if(!mPath.empty())
        //    break;

    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();


    if(!mPath.empty())
    {
        deltaTime = 0.0f;
        lastFrame = 0.0f;
        // return openEditor();
    }

    return EXIT_SUCCESS;
}
