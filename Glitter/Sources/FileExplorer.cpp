#include "UI/FileExplorer.hpp"
#include <imgui.h>
#include "state.hpp"
#include <filesystem>
namespace fs = std::filesystem;

void ProjectAsset::RenderFileExplorer(
                std::string currentPath,
                std::vector<std::string> fileNames)
{
    if (ImGui::Begin("File Dialog", &getUIState().showFileExplorerWindow))
    {

        for (const auto& entry : fs::directory_iterator(currentPath))
        {
            fileNames.push_back(entry.path().string());
        }

        // Display the current path
        ImGui::Text("Current Path: %s", currentPath.c_str());
        
        if(ImGui::Button("Go to Root of project"))
        {
            currentPath = State::state->projectRootLocation;
        }

        if(State::state->errorStack.size() != 0)
        ImGui::TextColored(ImVec4(1,0,0,1), State::state->errorStack.LastElement().c_str());

        // Up button to go to the parent directory
        if (ImGui::Button("Up"))
        {
            currentPath = fs::path(currentPath).parent_path().string();
        }

        // List files and directories
        if (ImGui::BeginListBox("##FileList", ImVec2(-FLT_MIN, 100)))
        {
            for (unsigned int fileIndex = 0; fileIndex < fileNames.size(); fileIndex++)
            {
                const bool isSelected = (getUIState().selectedFileIndex == fileIndex);
                if (ImGui::Selectable(fileNames[fileIndex].c_str(), isSelected))
                {
                    getUIState().selectedFileIndex = fileIndex;

                    // If a directory is selected, navigate into it
                    if (fs::is_directory(fileNames[fileIndex]))
                    {
                        currentPath = fileNames[fileIndex];
                    }
                }
            }
            ImGui::EndListBox();
        }
        ImGui::End();
    }
}