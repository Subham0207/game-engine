#include <UI/ProjectManager.hpp>
#include <imgui.h>
#include <UI/FileExplorer.hpp>
#include <iostream>
#include <utility>

ProjectManagerUI::ProjectManager::ProjectManager(std::string currentPath, std::string rootPath): mCurrentPath(std::move(currentPath)), mRootPath(std::move(rootPath))
{
    createANewProject = false;
    openAProject = false;
    newProjectName = "";

    mSelectedFileIndex = 0;
    mSelectedFilePath = "";
}

void ProjectManagerUI::ProjectManager::draw()
{
    if(ImGui::Begin("Project Manager"))
    {
        if(ImGui::Button("Create a new project"))
        {
            createANewProject = true;
        }


        ImGui::Text("Recent Projects:");
        ImGui::Separator();
        
        // Read the current list for display
        for (const auto& projectDir : recent_projects) {
            // Using Selectable to show that the items are interactive
            if (ImGui::Selectable(projectDir.string().c_str())) {
                // This is where you would handle opening the selected project
                currentActiveProjectDirectory = projectDir.string();
                std::cout << "Selected: " << projectDir << std::endl;
            }
        }

        ImGui::End();
    }

    if(createANewProject)
        ProjectAsset::createANewProject(
            mCurrentPath,
            fileNames,
            createANewProject,
            newProjectName,
            mRootPath,
            mSelectedFileIndex,
            mSelectedFilePath,
            recent_projects
        );

    if(openAProject)
        ProjectAsset::createANewProject(
            mCurrentPath,
            fileNames,
            createANewProject,
            newProjectName,
            mRootPath,
            mSelectedFileIndex,
            mSelectedFilePath,
            recent_projects
        );
    
}
