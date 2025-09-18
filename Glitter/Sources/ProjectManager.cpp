#include <UI/ProjectManager.hpp>
#include <imgui.h>
#include <EngineState.hpp>
#include <UI/FileExplorer.hpp>

void UI::projectManager()
{
    if(ImGui::Begin("Project Manager"))
    {
        if(ImGui::Button("Create a new project"))
        {
            getUIState().createANewProject = true;
        }


        ImGui::Text("Recent Projects:");
        ImGui::Separator();
        
        // Read the current list for display
        for (const auto& projectDir : getUIState().recent_projects) {
            // Using Selectable to show that the items are interactive
            if (ImGui::Selectable(projectDir.string().c_str())) {
                // This is where you would handle opening the selected project
                State::state->currentActiveProjectDirectory = projectDir.string();
                std::cout << "Selected: " << projectDir << std::endl;
            }
        }

        ImGui::End();
    }

    if(getUIState().createANewProject)
        ProjectAsset::createANewProject(
            getUIState().currentPath, State::state->uiState.fileNames, getUIState().createANewProject 
        );

    if(getUIState().openAProject)
        ProjectAsset::createANewProject(
            getUIState().currentPath, State::state->uiState.fileNames, getUIState().createANewProject 
        );
    
}