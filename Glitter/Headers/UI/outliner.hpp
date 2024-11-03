#pragma once
#include "imgui.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"
#include "EngineState.hpp"
#include "3DModel/Animation/Animator.hpp"

#include "assimp/scene.h"
#include "Character/Character.hpp"
#include "Level/Level.hpp"
#include <filesystem>
namespace fs = std::filesystem;


class Outliner 
{
public:
    // Constructor
    Outliner(std::vector<Model *> models) {
        // Initialize the items array or any other setup needed
        getUIState().models = models;
        getUIState().selectedModelIndex = -1;
    }

    // Render the radio buttons
    void Render(Level &lvl) {
        ImGui::Begin("Outliner");

        ImGui::Text("Current Level %s", lvl.levelname.c_str());
        if(ImGui::Button("Save Level"))
        {
            Level::saveToFile(lvl.levelname, lvl);
        }
        if(ImGui::Button("Save Level as"))
        {
            //POP up another window to give the lvl file a name and save perhaps ?
            //give level a name
            //choose the directory
            getUIState().saveAsFileName = "";
            getUIState().showFileDialog = true;
            getUIState().showOpenButton = false;
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::saveLevelAs;
        }


        ImGui::Text("Coordinate space");
        if (ImGui::RadioButton("World space",&getUIState().coordinateSpace,0)) {
            State::state->isWorldSpace = true;
        }
        if (ImGui::RadioButton("Local space",&getUIState().coordinateSpace,1)) {
            State::state->isWorldSpace = true;
        }

        ImGui::NewLine();

        for (int i = 0; i < getUIState().models.size(); ++i) {
            // Optionally, push style changes here if you want to customize appearance
            // Render the radio button
            // The label for each button could be customized further if needed
            std::string name = (getUIState().models)[i]->getName();
            name+=std::to_string(i);
            if (ImGui::RadioButton( name.c_str(), &getUIState().selectedModelIndex, i)) {
            }
            // Optionally, pop style changes here if you made any
        }

        ImGui::NewLine();

        //Show transformation of the selected model
        ImGui::PushItemWidth(40);
        if(getUIState().selectedModelIndex > -1)
        {
            glm::mat4& modelMatrix = (getUIState().models)[getUIState().selectedModelIndex]->model;
            glm::vec3 scale, rotation, translation, skew;
            glm::vec4 perspective;
            glm::quat orientation;
            
            //Decompose the selected model matrix into translation, rotation and scale vectors
            if (glm::decompose(modelMatrix, scale, orientation, translation, skew, perspective)) 
            {
                // Convert from radians to degrees if needed
                rotation = glm::degrees(glm::eulerAngles(orientation));

                ImGui::Text("Translation");
                ImGui::DragFloat("X##translation", &translation.x, 0.005f);
                ImGui::SameLine();
                ImGui::DragFloat("Y##translation", &translation.y, 0.005f);
                ImGui::SameLine();
                ImGui::DragFloat("Z##translation", &translation.z, 0.005f);

                // ImGui interface for scale
                ImGui::Text("Scale");
                ImGui::DragFloat("X##scale", &scale.x, 0.005f, 0.1f, 100.0f);  // Clamped to reasonable values
                ImGui::SameLine();
                ImGui::DragFloat("Y##scale", &scale.y, 0.005f, 0.1f, 100.0f);
                ImGui::SameLine();
                ImGui::DragFloat("Z##scale", &scale.z, 0.005f, 0.1f, 100.0f);

                // // // ImGui interface for rotation
                ImGui::Text("Rotation");
                ImGui::DragFloat("X##rotation", &rotation.x, 0.1f);
                ImGui::SameLine();
                ImGui::DragFloat("Y##rotation", &rotation.y, 0.1f);
                ImGui::SameLine();
                ImGui::DragFloat("Z##rotation", &rotation.z, 0.1f);
                
                // glm::quat combinedRotation = orientation * newRotation;
                modelMatrix = glm::recompose(scale, glm::quat(glm::radians(rotation)), translation, skew, perspective);

            } else 
            {
                std::cerr << "Failed to decompose the the matrix" << std::endl;
            }
        }
        if(ImGui::Button("Load a level"))
        {
            getUIState().showFileDialog = true;
            getUIState().showOpenButton = true;
            getUIState().fileExtension = ".lvl";
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::LoadLvlFile;
        }
        if(ImGui::Button("Import a Model"))
        {
            getUIState().loadModelWindow = true;
        }
        if(ImGui::Button("Save a Model"))
        {
            //Get selected index of the model
            //open UI to enter name of the model
            //call saveModel function
            getUIState().showFileDialog = true;
            getUIState().showOpenButton = false;
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::saveModel;
        }
        if(ImGui::Button("Load a Model"))
        {
            getUIState().showFileDialog = true;
            getUIState().showOpenButton = true;
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::loadModel;
        }
        if(ImGui::Button("Load animation for model"))
        {
            getUIState().showFileDialog = true;
            getUIState().showOpenButton = true;
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::loadAnimation;
        }
        if (ImGui::Combo("Choose an option", &getUIState().selectedAnimationIndex, getUIState().animationNames.data(),getUIState().animationNames.size())) {
            // Action when the selection changes
            // std::cout << "Selected: " << animationNames[current_item] << std::endl;
        }
        if(ImGui::Button("Play AnimationType"))
        {
            if(getUIState().character->animator != nullptr)
            getUIState().character->animator->PlayAnimation(getUIState().animations[getUIState().selectedAnimationIndex]);
            // we need to send the manipulation to mesh
        }
        if(ImGui::Button("Increment selected bone"))
        {
            if(getUIState().character != nullptr)
            {
                getUIState().selectedBoneId++;
            }
        }
        if(ImGui::Button("Reset selected bone"))
        {
            if(getUIState().character != nullptr)
            {
                getUIState().selectedBoneId = 0;
            }
        }
        
        if (ImGui::BeginPopupModal("Warning", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text(State::state->errorStack.LastElement().c_str());
            ImGui::Separator();

            if (ImGui::Button("OK")) { 
                State::state->errorStack.LastElement() = "";
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if(State::state->errorStack.size() != 0)
        ImGui::OpenPopup("Warning");
        ImGui::End();
        

        if(getUIState().showFileDialog)
        {
            selectAFile(lvl);
        }

        if(getUIState().loadModelWindow)
        {
            ModelAndTextureSelectionWindow();
        }

        if(getUIState().isFirstFrame){
        ImGui::SetWindowFocus(false);
        getUIState().isFirstFrame = false;
        }
    }

    // Get the index of the currently selected radio button
    int GetSelectedIndex() const {
        return getUIState().selectedModelIndex;
    }
    void setSelectedIndex(int newSelectedIndex){
    getUIState().selectedModelIndex = newSelectedIndex;
    }

    void applyRotation(glm::mat4& modelMatrix, glm::vec3 rotationDegrees, bool isLocal) 
    {
        // Convert degrees to radians for rotation
        glm::vec3 rotationRadians = glm::radians(rotationDegrees);

        // Prepare rotation quaternions based on the input
        glm::quat quatX = glm::angleAxis(rotationRadians.x, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat quatY = glm::angleAxis(rotationRadians.y, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat quatZ = glm::angleAxis(rotationRadians.z, glm::vec3(0.0f, 0.0f, 1.0f));

        // Combine rotations
        glm::quat combinedQuat = quatZ * quatY * quatX;

        // Calculate rotation matrix
        glm::mat4 rotationMatrix = glm::mat4_cast(combinedQuat);

        // Apply rotation
        if (isLocal) {
            // Local rotation
            modelMatrix *= rotationMatrix;
        } else {
            // World rotation: rotate around global axes
            // Reset the translation part to avoid moving the object
            glm::vec3 translation = glm::vec3(modelMatrix[3]);
            modelMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            modelMatrix = rotationMatrix * modelMatrix;
            modelMatrix[3] = glm::vec4(translation, 1.0f);
        }
    }

    void selectA3DModel(Level &level)
    {

    }

    void selectAFile(Level &level)
    {
        std::vector<std::string> fileNames;
        static std::string currentPath = fs::current_path().string();

        
        if (ImGui::Begin("File Dialog", &getUIState().showFileDialog))
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
                for (const auto& file : fileNames)
                {
                    const bool isSelected = (getUIState().filePath == file);
                    if (ImGui::Selectable(file.c_str(), isSelected))
                    {
                        getUIState().filePath = file;

                        // If a directory is selected, navigate into it
                        if (fs::is_directory(file))
                        {
                            currentPath = file;
                        }
                    }
                }
                ImGui::EndListBox();
            }

            // Show selected file path
            if(getUIState().showOpenButton)
            ImGui::Text("Selected File: %s", getUIState().filePath.c_str());

            // Open file button
            if(getUIState().showOpenButton)
            {
                if (ImGui::Button("Open"))
                {
                    if (!fs::is_directory(getUIState().filePath))
                    {
                        switch (getUIState().fileTypeOperation)
                        {
                            case ProjectAsset::FileTypeOperation::LoadLvlFile:
                                {
                                    Level::loadFromFile(getUIState().filePath, level);
                                    getUIState().showFileDialog = false;
                                }
                                break;
                            case ProjectAsset::FileTypeOperation::importModelFile:
                                {
                                    getUIState().modelfileName = getUIState().filePath;
                                    getUIState().character = new Character();
                                    getUIState().character->model = new Model(const_cast<char*>(getUIState().filePath.c_str()));
                                    level.addModel(getUIState().character->model);    
                                    getUIState().showFileDialog = false;                       
                                }
                                break;
                            case ProjectAsset::FileTypeOperation::albedoTexture:
                                {
                                    getUIState().albedo = getUIState().filePath;
                                    getUIState().character->model->LoadTexture(getUIState().filePath, aiTextureType_DIFFUSE);
                                    getUIState().showFileDialog = false;                      
                                }
                                break;
                            case ProjectAsset::FileTypeOperation::normalTexture:
                                {
                                    getUIState().normal = getUIState().filePath;
                                    getUIState().character->model->LoadTexture(getUIState().filePath, aiTextureType_NORMALS);
                                    getUIState().showFileDialog = false;                      
                                }
                                break;
                            case ProjectAsset::FileTypeOperation::metalnessTexture:
                                {
                                    getUIState().metalness = getUIState().filePath;
                                    getUIState().character->model->LoadTexture(getUIState().filePath, aiTextureType_METALNESS);
                                    getUIState().showFileDialog = false;                      
                                }
                                break;
                            case ProjectAsset::FileTypeOperation::roughnessTexture:
                                {
                                    getUIState().roughness = getUIState().filePath;
                                    getUIState().character->model->LoadTexture(getUIState().filePath, aiTextureType_DIFFUSE_ROUGHNESS);
                                    getUIState().showFileDialog = false;                      
                                }
                                break;
                            case ProjectAsset::FileTypeOperation::aoTexture:
                                {
                                    getUIState().ao = getUIState().filePath;
                                    getUIState().character->model->LoadTexture(getUIState().filePath, aiTextureType_AMBIENT_OCCLUSION);
                                    getUIState().showFileDialog = false;                      
                                }
                                break;
                            case ProjectAsset::FileTypeOperation::loadModel:
                                {
                                    getUIState().character->model = new Model();
                                    Model::loadFromFile(getUIState().filePath, *getUIState().character->model);
                                    level.addModel(getUIState().character->model);
                                    getUIState().showFileDialog = false;                      
                                }
                                break;
                            case ProjectAsset::FileTypeOperation::loadAnimation:
                                {
                                    if(getUIState().character != NULL)
                                    {
                                        auto animation = new Animation(getUIState().filePath, getUIState().character->model);
                                        getUIState().animations.push_back(animation);
                                        getUIState().animationNames.push_back(animation->animationName);
                                    }
                                    else
                                    {
                                        State::state->errorStack.LastElement() = "Please first Load a model and have it selected before loading an animation";
                                    }
                                    getUIState().showFileDialog = false;                      
                                }
                                break;
                        
                        default:
                            break;
                        }
                    }
                }

            }

            if(!getUIState().showOpenButton)
            {
                switch(getUIState().fileTypeOperation)
                {
                    case ProjectAsset::FileTypeOperation::saveLevelAs: {
                        InputText("##Filename", getUIState().saveAsFileName);
                        if (ImGui::Button("Save"))
                        {
                            level.levelname = getUIState().saveAsFileName;
                            Level::saveToFile("Assets/" + getUIState().saveAsFileName, level);
                            getUIState().showFileDialog = false;
                        }
                    }
                    break;

                    case ProjectAsset::FileTypeOperation::saveModel: {
                        InputText("##Filename", getUIState().saveAsFileName);
                        if (ImGui::Button("Save"))
                        {
                            auto model = getUIState().models[getUIState().selectedModelIndex];
                            Model::saveSerializedModel("Assets/" + getUIState().saveAsFileName, *model);
                            //Recurrsively call save texture on the texture method ?
                            getUIState().showFileDialog = false;
                        }
                    }
                    break;
                };
            }

            ImGui::End();
        }
    }

    void ModelAndTextureSelectionWindow()
    {
        if(ImGui::Begin("Import ModelType", &getUIState().loadModelWindow))
        {   
            ImGui::Text("ModelType");
            ImGui::SameLine();
            ImGui::Text("%s", getUIState().modelfileName.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##1"))
            {
                getUIState().showFileDialog = true;
                getUIState().showOpenButton = true;
                getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::importModelFile;
            }

            ImGui::Text("Albedo");
            ImGui::SameLine();
            ImGui::Text("%s", getUIState().albedo.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##2"))
            {
                getUIState().showFileDialog = true;
                getUIState().showOpenButton = true;
                getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::albedoTexture;
            }

            ImGui::Text("Normal");
            ImGui::SameLine();
            ImGui::Text("%s", getUIState().normal.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##3"))
            {
                getUIState().showFileDialog = true;
                getUIState().showOpenButton = true;
                getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::normalTexture;
            }

            ImGui::Text("Metalness");
            ImGui::SameLine();
            ImGui::Text("%s", getUIState().metalness.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##5"))
            {
                getUIState().showFileDialog = true;
                getUIState().showOpenButton = true;
                getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::metalnessTexture;
            }

            ImGui::Text("Roughness");
            ImGui::SameLine();
            ImGui::Text("%s", getUIState().roughness.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##6"))
            {
                getUIState().showFileDialog = true;
                getUIState().showOpenButton = true;
                getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::roughnessTexture;
            }

            ImGui::Text("AO");
            ImGui::SameLine();
            ImGui::Text("%s", getUIState().ao.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##7"))
            {
                getUIState().showFileDialog = true;
                getUIState().showOpenButton = true;
                getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::aoTexture;
            }

            ImGui::End();
        }
    }

    void updateAnimator(float deltatime)
    {
        if(getUIState().selectedAnimationIndex != -1 && getUIState().character != nullptr && getUIState().animations.size() != 0)
        getUIState().character->animator->UpdateAnimation(deltatime);
    }

    void updateFinalBoneMatrix(Shader ourShader)
    {
        getUIState().shaderOfSelectedModel = &ourShader;
        if(getUIState().character->animator != nullptr)
        {
            auto transforms = getUIState().character->animator->GetFinalBoneMatrices();
            for (int i = 0; i < transforms.size(); ++i)
                ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }
        else{
            //MAXBONES 100
                for (int i = 0; i < 100; ++i)
                ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", 1.0f);
        }
        if(getUIState().character->model != nullptr)
        {
            getUIState().shaderOfSelectedModel->setInt("displayBoneIndex", getUIState().selectedBoneId);
        }
    }

private:
    bool InputText(const char* label, std::string& str, ImGuiInputTextFlags flags = 0) {
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
};
