#pragma once
#include "UI/FileExplorer.hpp"
#include <imgui.h>
#include "EngineState.hpp"
#include <EngineState.hpp>
#include <Character/Character.hpp>
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
        currentPath = State::state->projectRootLocation;
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
                    Level::saveToFile("Assets/" + getUIState().saveAsFileName, getActiveLevel());
                    showUI = false;
                }
            }
            break;

            case FileTypeOperation::saveModel: {
                InputText("##Filename", getUIState().saveAsFileName);
                if (ImGui::Button("Save"))
                {
                    auto model = getUIState().models.at(getUIState().selectedModelIndex);
                    Model::saveSerializedModel("Assets/" + getUIState().saveAsFileName, *model);
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
                            getUIState().character = new Character(getUIState().filePath);
                            getActiveLevel().addModel(getUIState().character->model);
                            getUIState().models = *State::state->activeLevel.models;    
                            showUI = false;                       
                        }
                        break;
                    case FileTypeOperation::albedoTexture:
                        {
                            auto texture = getUIState().character->model->LoadTexture(getUIState().filePath, aiTextureType_DIFFUSE);
                            getUIState().materials[getUIState().materialIndex]->albedo = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::normalTexture:
                        {
                            auto texture = getUIState().character->model->LoadTexture(getUIState().filePath, aiTextureType_NORMALS);
                            getUIState().materials[getUIState().materialIndex]->normal = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::metalnessTexture:
                        {
                            auto texture = getUIState().character->model->LoadTexture(getUIState().filePath, aiTextureType_METALNESS);
                            getUIState().materials[getUIState().materialIndex]->metalness = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::roughnessTexture:
                        {
                            auto texture = getUIState().character->model->LoadTexture(getUIState().filePath, aiTextureType_DIFFUSE_ROUGHNESS);
                            getUIState().materials[getUIState().materialIndex]->roughness = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::aoTexture:
                        {
                            auto texture = getUIState().character->model->LoadTexture(getUIState().filePath, aiTextureType_AMBIENT_OCCLUSION);
                            getUIState().materials[getUIState().materialIndex]->ao = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::loadModel:
                        {
                            getUIState().character->model = new Model();
                            Model::loadFromFile(getUIState().filePath, *getUIState().character->model);
                            getActiveLevel().addModel(getUIState().character->model);
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::loadAnimation:
                        {
                            if(getUIState().character != NULL)
                            {
                                auto animation = new Animation(
                                getUIState().filePath,
                                getUIState().character->GetBoneInfoMap(),
                                getUIState().character->GetBoneCount());
                                getUIState().animations.push_back(animation);
                                getUIState().animationNames.push_back(animation->animationName);
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

bool ProjectAsset::InputText(const char* label, std::string& str, ImGuiInputTextFlags flags) {
        // Ensure the buffer is large enough to hold the text
        char buffer[256];
        std::strncpy(buffer, str.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        // Create the InputText widget
        bool result = ImGui::InputText(label, buffer, sizeof(buffer), flags);

        // Update the std::string if the text was modified
        if (result) {
            str = std::string(buffer);
        }

        return result;
    }