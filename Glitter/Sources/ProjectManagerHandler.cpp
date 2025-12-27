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
#include "Lights/light.hpp"


#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <vector>

#include "UI/outliner.hpp"

#include <EngineState.hpp>
#include "Level/Level.hpp"
#include <Helpers/Shared.hpp>

#include <UI/ProjectManager.hpp>

#include "Controls/ClientHandler.hpp"

int ProjectManagerHandler::startProjectManager()
{
    EngineState::state = new EngineState();
    ClientHandler::clientHandler = new ClientHandler();

    if (fs::exists(fs::path(EngineState::state->engineInstalledDirectory) / "user_prefs.json")) {
        std::ifstream infile(fs::path(EngineState::state->engineInstalledDirectory) / "user_prefs.json");
        std::string line;
        while (std::getline(infile, line)) {
            // Check if the line is not empty before adding.
            if (!line.empty()) {
                getUIState().recent_projects.push_back(fs::path(line));
            }
        }
        infile.close();
    }

    EngineState::state->mWindow = Shared::initAWindow();

    Shared::initImguiBackend(EngineState::state->mWindow);

    while (glfwWindowShouldClose(EngineState::state->mWindow) == false)
    {

        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //////Code changes go here//////
        UI::projectManager();
        ////////////////////////////////

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(EngineState::state->mWindow);
        glfwPollEvents();

        // Now it's safe to leave the loop
        if(!EngineState::state->currentActiveProjectDirectory.empty())
            break;

    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();


    if(!EngineState::state->currentActiveProjectDirectory.empty())
    {
        EngineState::state->deltaTime = 0.0f;
        EngineState::state->lastFrame = 0.0f;
        // return openEditor();
    }

    return EXIT_SUCCESS;
}
