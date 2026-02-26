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
        currentPath = EngineState::state->currentActiveProjectDirectory;
    }

    if(EngineState::state->errorStack.size() != 0)
    ImGui::TextColored(ImVec4(1,0,0,1), EngineState::state->errorStack.LastElement().c_str());

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
                    
                    getActiveLevel().save(
                        fs::path(EngineState::state->currentActiveProjectDirectory) / "Levels");
                    showUI = false;
                }
            }
            break;

            case FileTypeOperation::saveModel: {
                InputText("##Filename", getUIState().saveAsFileName);
                if (ImGui::Button("Save") && getUIState().selectedRenderableIndex > -1)
                {
                    auto renderable = getActiveLevel().renderables.at(getUIState().selectedRenderableIndex);
                    if(auto model = std::dynamic_pointer_cast<Model>(renderable))
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
                            auto level = new Level();
                            level->load(fs::path(getUIState().filePath), "main.lvl");
                            showUI = false;
                        }
                        break;
                    case FileTypeOperation::importModelFile:
                        {
                            //Create .model file.
                            //If skeleton data is there then create .skeleton file.
                            //Then we can create character entity obj file and use the skeleton.
                            Assimp::Importer import;
                            auto scene = AssimpHelpers::createAssimpScene(getUIState().filePath, import);
                            bool skinned = AssimpHelpers::isSkinned(scene);
                            auto path = fs::path(EngineState::state->currentActiveProjectDirectory) / "Assets";

                            getUIState().modelfileName = getUIState().filePath;

                            if (skinned)
                            {
                                const auto skeleton = std::make_shared<Skeleton::Skeleton>();
                                skeleton->setup(getUIState().filePath);
                                const auto model = new Model();
                                model->LoadA3DModel(
                                    scene,
                                    true,
                                    getUIState().filePath,
                                    &skeleton->m_BoneInfoMap,
                                    &skeleton->m_BoneCounter);

                                model->setModelMatrix(glm::identity<glm::mat4>());

                                Skeleton::Skeleton::ReadHierarchyData(skeleton->m_RootNode, scene->mRootNode);
                                Helpers::resolveBoneHierarchy(scene->mRootNode, -1, skeleton->m_BoneInfoMap, skeleton->m_Bones);
                                skeleton->BuildBoneHierarchy();

                                skeleton->save(path);
                                model->save(path);

                                delete model;
                            }
                            else
                            {
                                const auto model = new Model();
                                model->LoadA3DModel(
                                    scene,
                                    false,
                                    getUIState().filePath,
                                    nullptr,
                                    nullptr);

                                model->setModelMatrix(glm::identity<glm::mat4>());

                                model->save(path);

                                delete model;
                            }
                            showUI = false;
                        }
                        break;
                    case FileTypeOperation::importCharacterFile:
                        {
                            getUIState().modelfileName = getUIState().filePath;
                            auto character = std::make_shared<Character>(getUIState().filePath);
                            character->setModelMatrix(glm::identity<glm::mat4>());
                            auto characterPath = fs::path(EngineState::state->currentActiveProjectDirectory) / "Assets";
                            character->save(characterPath);
                            getActiveLevel().addRenderable(character);
                            showUI = false;                       
                        }
                        break;
                    case FileTypeOperation::albedoTexture:
                        {
                            if(getUIState().selectedRenderableIndex < 0)
                            break;
                            auto texture = getActiveLevel().renderables[getUIState().selectedRenderableIndex]
                            ->LoadTexture(getUIState().filePath, aiTextureType_DIFFUSE);
                            getUIState().materials[getUIState().materialIndex]->GetTextureUnits().albedo = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::normalTexture:
                        {
                            if(getUIState().selectedRenderableIndex < 0)
                            break;
                            auto texture = getActiveLevel().renderables[getUIState().selectedRenderableIndex]
                            ->LoadTexture(getUIState().filePath, aiTextureType_NORMALS);
                            getUIState().materials[getUIState().materialIndex]->GetTextureUnits().normal = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::metalnessTexture:
                        {
                            if(getUIState().selectedRenderableIndex < 0)
                            break;
                            auto texture = getActiveLevel().renderables[getUIState().selectedRenderableIndex]
                            ->LoadTexture(getUIState().filePath, aiTextureType_METALNESS);
                            getUIState().materials[getUIState().materialIndex]->GetTextureUnits().metalness = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::roughnessTexture:
                        {
                            if(getUIState().selectedRenderableIndex < 0)
                            break;
                            auto texture = getActiveLevel().renderables[getUIState().selectedRenderableIndex]
                            ->LoadTexture(getUIState().filePath, aiTextureType_DIFFUSE_ROUGHNESS);
                            getUIState().materials[getUIState().materialIndex]->GetTextureUnits().roughness = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::aoTexture:
                        {
                            if(getUIState().selectedRenderableIndex < 0)
                            break;
                            auto texture = getActiveLevel().renderables[getUIState().selectedRenderableIndex]
                            ->LoadTexture(getUIState().filePath, aiTextureType_AMBIENT_OCCLUSION);
                            getUIState().materials[getUIState().materialIndex]->GetTextureUnits().ao = texture;
                            showUI = false;                      
                        }
                        break;
                    case FileTypeOperation::importAnimation:
                        {
                            auto projectAssetDirectory = fs::path(EngineState::state->currentActiveProjectDirectory) / "Assets";
                            auto animation = new Animation(getUIState().filePath);
                            animation->save(projectAssetDirectory);
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