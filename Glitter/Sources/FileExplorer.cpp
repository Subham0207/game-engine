#pragma once
#include "UI/FileExplorer.hpp"
#include <imgui.h>
#include <EngineState.hpp>
#include <Character/Character.hpp>
#include <Helpers/Shared.hpp>
#include <Helpers/createNewProject.hpp>
#include <filesystem>
namespace fs = std::filesystem;

void ProjectAsset::RenderFileExplorer(
                std::string& currentPath,
                std::vector<std::string>& fileNames)
{
    if(fileNames.size() == 0)
    for (const auto& entry : fs::directory_iterator(currentPath))
    {
        fileNames.push_back(entry.path().string());
    }

    // Display the current path
    ImGui::Text("Current Path: %s", currentPath.c_str());
    
    if(ImGui::Button("Go to Root of project"))
    {
        fileNames.clear();
        currentPath = State::state->currentActiveProjectDirectory;
    }

    if(State::state->errorStack.size() != 0)
    ImGui::TextColored(ImVec4(1,0,0,1), State::state->errorStack.LastElement().c_str());

    // Up button to go to the parent directory
    if (ImGui::Button("Up"))
    {
        fileNames.clear();
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

                // If a directory is selected, navigate into it else set selected file.
                if (fs::is_directory(fileNames[fileIndex]))
                {
                    currentPath = fileNames[fileIndex];
                    fileNames.clear();
                }else{
                    getUIState().filePath = fileNames[fileIndex];
                }

            }
        }
        ImGui::EndListBox();
    }
}

void ProjectAsset::saveAFile(std::string& currentPath,
                std::vector<std::string>& fileNames,
                bool& showUI)
{   
    if(ImGui::Begin("FileExplorer", &showUI))
    {
        ProjectAsset::RenderFileExplorer(currentPath, fileNames);
        switch(getUIState().fileTypeOperation)
        {
            case FileTypeOperation::saveLevelAs: {
                InputText("##Filename", getUIState().saveAsFileName);
                if (ImGui::Button("Save"))
                {
                    getActiveLevel().levelname = getUIState().saveAsFileName;
                    
                    Level::saveToFile(
                        (fs::path(State::state->currentActiveProjectDirectory) / "Levels" / getUIState().saveAsFileName).string() + ".lvl"
                        , getActiveLevel());
                    showUI = false;
                }
            }
            break;

            case FileTypeOperation::saveModel: {
                InputText("##Filename", getUIState().saveAsFileName);
                if (ImGui::Button("Save") && getUIState().selectedRenderableIndex > -1)
                {
                    auto renderable = getUIState().renderables.at(getUIState().selectedRenderableIndex);
                    if(auto* model = dynamic_cast<Model*>(renderable))
                    {
                        Model::saveSerializedModel("Assets/" + getUIState().saveAsFileName, *model);
                    }
                    //Recurrsively call save texture on the texture method ?
                    showUI = false;
                }
            }
            break;
        };
    ImGui::End();
    }
}

void ProjectAsset::selectOrLoadAFileFromFileExplorer(
                std::string& currentPath,
                std::vector<std::string>& fileNames,
                bool& showUI){
    if(ImGui::Begin("FileExplorer", &showUI))
    {
        ProjectAsset::RenderFileExplorer(currentPath, fileNames);

        ImGui::Text("Selected File: %s", getUIState().filePath.c_str());
        if (ImGui::Button("Open"))
        {
            if (!fs::is_directory(getUIState().filePath))
            {
                switch (getUIState().fileTypeOperation)
                {
                    case FileTypeOperation::LoadLvlFile:
                        {
                            Level::loadFromFile(getUIState().filePath, getActiveLevel());
                            showUI = false;
                        }
                        break;
                    case FileTypeOperation::importModelFile:
                        {
                            getUIState().modelfileName = getUIState().filePath;
                            getActiveLevel().addRenderable(new Character(getUIState().filePath));
                            getUIState().renderables = *State::state->activeLevel->renderables;    
                            showUI = false;                       
                        }
                        break;
                    case FileTypeOperation::albedoTexture:
                        {
                            if(getUIState().selectedRenderableIndex < 0)
                            break;
                            auto texture = getUIState().renderables[getUIState().selectedRenderableIndex]
                            ->LoadTexture(getUIState().filePath, aiTextureType_DIFFUSE);
                            getUIState().materials[getUIState().materialIndex]->albedo = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::normalTexture:
                        {
                            if(getUIState().selectedRenderableIndex < 0)
                            break;
                            auto texture = getUIState().renderables[getUIState().selectedRenderableIndex]
                            ->LoadTexture(getUIState().filePath, aiTextureType_NORMALS);
                            getUIState().materials[getUIState().materialIndex]->normal = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::metalnessTexture:
                        {
                            if(getUIState().selectedRenderableIndex < 0)
                            break;
                            auto texture = getUIState().renderables[getUIState().selectedRenderableIndex]
                            ->LoadTexture(getUIState().filePath, aiTextureType_METALNESS);
                            getUIState().materials[getUIState().materialIndex]->metalness = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::roughnessTexture:
                        {
                            if(getUIState().selectedRenderableIndex < 0)
                            break;
                            auto texture = getUIState().renderables[getUIState().selectedRenderableIndex]
                            ->LoadTexture(getUIState().filePath, aiTextureType_DIFFUSE_ROUGHNESS);
                            getUIState().materials[getUIState().materialIndex]->roughness = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::aoTexture:
                        {
                            if(getUIState().selectedRenderableIndex < 0)
                            break;
                            auto texture = getUIState().renderables[getUIState().selectedRenderableIndex]
                            ->LoadTexture(getUIState().filePath, aiTextureType_AMBIENT_OCCLUSION);
                            getUIState().materials[getUIState().materialIndex]->ao = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::loadModel:
                        {
                            auto renderable = new Model();
                            Model::loadFromFile(getUIState().filePath, *renderable);
                            getActiveLevel().addRenderable(renderable);
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::loadAnimation:
                        {
                            if(getUIState().selectedRenderableIndex < 0)
                            break;
                            if(auto character = dynamic_cast<Character *>(getUIState().renderables[getUIState().selectedRenderableIndex]))
                            {
                                std::string& filename = getUIState().filePath;
                                Shared::readAnimation(filename);
                            }
                            else
                            {
                                State::state->errorStack.LastElement() = "Please first Load a model and have it selected before loading an animation";
                            }
                            showUI = false;                      
                        }
                        break;
                
                default:
                    break;
                }
            }
        }
        ImGui::End();
    }
}

void ProjectAsset::createANewProject(
    std::string& currentPath,
    std::vector<std::string>& fileNames,
    bool &showUI)
{
    if(ImGui::Begin("FileExplorer", &showUI))
    {
        ProjectAsset::RenderFileExplorer(currentPath, fileNames);
        InputText("##NEW_PROJECT_NAME", getUIState().newProjectName);

        if (ImGui::Button("Save"))
        {
            create_new_project(
                currentPath,
                getUIState().newProjectName);
            showUI = false;
        }

        ImGui::End();
    }
}

void ProjectAsset::openAProject(std::string &currentPath, std::vector<std::string> &fileNames, bool &showUI)
{
    if(ImGui::Begin("FileExplorer", &showUI))
    {
        ProjectAsset::RenderFileExplorer(currentPath, fileNames);
        InputText("##NEW_PROJECT_NAME", getUIState().newProjectName);

        if (ImGui::Button("Save"))
        {
            create_new_project(
                currentPath,
                getUIState().newProjectName);
            showUI = false;
        }

        ImGui::End();
    }
}
bool ProjectAsset::InputText(const char *label, std::string &str, ImGuiInputTextFlags flags)
{
    // Ensure the buffer is large enough to hold the text
    char buffer[256];
    std::strncpy(buffer, str.c_str(), sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    // Create the InputText widget
    bool result = ImGui::InputText(label, buffer, sizeof(buffer), flags);

    // Update the std::string if the text was modified
    if (result)
    {
        str = std::string(buffer);
    }

    return result;
}