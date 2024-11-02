#pragma once
#include "imgui.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"
#include "state.hpp"
#include "3DModel/Animation/Animator.hpp"

#include "assimp/scene.h"
#include "Character/Character.hpp"
#include "Level/Level.hpp"
#include <filesystem>
namespace fs = std::filesystem;

enum FileTypeOperation {
    LoadLvlFile, importModelFile, saveModel, loadModel,albedoTexture, normalTexture, metalnessTexture, roughnessTexture, aoTexture, saveLevel, saveLevelAs,
    loadAnimation
};

class Outliner 
{
public:
    // Constructor
    Outliner(std::vector<Model *> *models) : mModels(models), selectedModelIndex(-1) {
        // Initialize the items array or any other setup needed
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
            saveAsFileName = "";
            showFileDialog = true;
            showOpenButton = false;
            fileTypeOperation = FileTypeOperation::saveLevelAs;
        }


        ImGui::Text("Coordinate space");
        if (ImGui::RadioButton("World space",&coordinateSpace,0)) {
            State::state->isWorldSpace = true;
        }
        if (ImGui::RadioButton("Local space",&coordinateSpace,1)) {
            State::state->isWorldSpace = true;
        }

        ImGui::NewLine();

        for (int i = 0; i < mModels->size(); ++i) {
            // Optionally, push style changes here if you want to customize appearance
            // Render the radio button
            // The label for each button could be customized further if needed
            std::string name = (*mModels)[i]->getName();
            name+=std::to_string(i);
            if (ImGui::RadioButton( name.c_str(), &selectedModelIndex, i)) {
            }
            // Optionally, pop style changes here if you made any
        }

        ImGui::NewLine();

        //Show transformation of the selected model
        ImGui::PushItemWidth(40);
        if(selectedModelIndex > -1)
        {
            glm::mat4& modelMatrix = (*mModels)[selectedModelIndex]->model;
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
            showFileDialog = true;
            showOpenButton = true;
            fileExtension = ".lvl";
            fileTypeOperation = FileTypeOperation::LoadLvlFile;
        }
        if(ImGui::Button("Import a Model"))
        {
            loadModelWindow = true;
        }
        if(ImGui::Button("Save a Model"))
        {
            //Get selected index of the model
            //open UI to enter name of the model
            //call saveModel function
            showFileDialog = true;
            showOpenButton = false;
            fileTypeOperation = FileTypeOperation::saveModel;
        }
        if(ImGui::Button("Load a Model"))
        {
            showFileDialog = true;
            showOpenButton = true;
            fileTypeOperation = FileTypeOperation::loadModel;
        }
        if(ImGui::Button("Load animation for model"))
        {
            showFileDialog = true;
            showOpenButton = true;
            fileTypeOperation = FileTypeOperation::loadAnimation;
        }
        if (ImGui::Combo("Choose an option", &selectedAnimationIndex, animationNames.data(),animationNames.size())) {
            // Action when the selection changes
            // std::cout << "Selected: " << animationNames[current_item] << std::endl;
        }
        if(ImGui::Button("Play Animation"))
        {
            if(character->animator != nullptr)
            character->animator->PlayAnimation(animations[selectedAnimationIndex]);
            // we need to send the manipulation to mesh
        }
        if(ImGui::Button("Increment selected bone"))
        {
            if(character != nullptr)
            {
                selectedBoneId++;
            }
        }
        if(ImGui::Button("Reset selected bone"))
        {
            if(character != nullptr)
            {
                selectedBoneId = 0;
            }
        }
        
        if (ImGui::BeginPopupModal("Warning", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text(errorMessage.c_str());
            ImGui::Separator();

            if (ImGui::Button("OK")) { 
                errorMessage = "";
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if(errorMessage != "")
        ImGui::OpenPopup("Warning");
        ImGui::End();
        

        if(showFileDialog)
        {
            selectAFile(lvl);
        }

        if(loadModelWindow)
        {
            ModelAndTextureSelectionWindow();
        }

        if(isFirstFrame){
        ImGui::SetWindowFocus(false);
        isFirstFrame = false;
        }
    }

    // Get the index of the currently selected radio button
    int GetSelectedIndex() const {
        return selectedModelIndex;
    }
    void setSelectedIndex(int newSelectedIndex){
    selectedModelIndex = newSelectedIndex;
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

        
        if (ImGui::Begin("File Dialog", &showFileDialog))
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

            if(error != "")
            ImGui::TextColored(ImVec4(1,0,0,1), error.c_str());

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
                    const bool isSelected = (filePath == file);
                    if (ImGui::Selectable(file.c_str(), isSelected))
                    {
                        filePath = file;

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
            if(showOpenButton)
            ImGui::Text("Selected File: %s", filePath.c_str());

            // Open file button
            if(showOpenButton)
            {
                if (ImGui::Button("Open"))
                {
                    if (!fs::is_directory(filePath))
                    {
                        switch (fileTypeOperation)
                        {
                            case FileTypeOperation::LoadLvlFile:
                                {
                                    Level::loadFromFile(filePath, level);
                                    showFileDialog = false;
                                }
                                break;
                            case FileTypeOperation::importModelFile:
                                {
                                    modelfileName = filePath;
                                    character = new Character();
                                    character->model = new Model(const_cast<char*>(filePath.c_str()));
                                    level.addModel(character->model);    
                                    showFileDialog = false;                       
                                }
                                break;
                            case FileTypeOperation::albedoTexture:
                                {
                                    albedo = filePath;
                                    character->model->LoadTexture(filePath, aiTextureType_DIFFUSE);
                                    showFileDialog = false;                      
                                }
                                break;
                            case FileTypeOperation::normalTexture:
                                {
                                    normal = filePath;
                                    character->model->LoadTexture(filePath, aiTextureType_NORMALS);
                                    showFileDialog = false;                      
                                }
                                break;
                            case FileTypeOperation::metalnessTexture:
                                {
                                    metalness = filePath;
                                    character->model->LoadTexture(filePath, aiTextureType_METALNESS);
                                    showFileDialog = false;                      
                                }
                                break;
                            case FileTypeOperation::roughnessTexture:
                                {
                                    roughness = filePath;
                                    character->model->LoadTexture(filePath, aiTextureType_DIFFUSE_ROUGHNESS);
                                    showFileDialog = false;                      
                                }
                                break;
                            case FileTypeOperation::aoTexture:
                                {
                                    ao = filePath;
                                    character->model->LoadTexture(filePath, aiTextureType_AMBIENT_OCCLUSION);
                                    showFileDialog = false;                      
                                }
                                break;
                            case FileTypeOperation::loadModel:
                                {
                                    character->model = new Model();
                                    Model::loadFromFile(filePath, *character->model);
                                    level.addModel(character->model);
                                    showFileDialog = false;                      
                                }
                                break;
                            case FileTypeOperation::loadAnimation:
                                {
                                    if(character != NULL)
                                    {
                                        auto animation = new Animation(filePath, character->model);
                                        animations.push_back(animation);
                                        animationNames.push_back(animation->animationName);
                                    }
                                    else
                                    {
                                        errorMessage = "Please first Load a model and have it selected before loading an animation";
                                    }
                                    showFileDialog = false;                      
                                }
                                break;
                        
                        default:
                            break;
                        }
                    }
                }

            }

            if(!showOpenButton)
            {
                switch(fileTypeOperation)
                {
                    case FileTypeOperation::saveLevelAs: {
                        InputText("##Filename", saveAsFileName);
                        if (ImGui::Button("Save"))
                        {
                            level.levelname = saveAsFileName;
                            Level::saveToFile("Assets/" + saveAsFileName, level);
                            showFileDialog = false;
                        }
                    }
                    break;

                    case FileTypeOperation::saveModel: {
                        InputText("##Filename", saveAsFileName);
                        if (ImGui::Button("Save"))
                        {
                            auto model = mModels->at(selectedModelIndex);
                            Model::saveSerializedModel("Assets/" + saveAsFileName, *model);
                            //Recurrsively call save texture on the texture method ?
                            showFileDialog = false;
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
        if(ImGui::Begin("Load Model", &loadModelWindow))
        {   
            ImGui::Text("Model");
            ImGui::SameLine();
            ImGui::Text("%s", modelfileName.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##1"))
            {
                showFileDialog = true;
                showOpenButton = true;
                fileTypeOperation = FileTypeOperation::importModelFile;
            }

            ImGui::Text("Albedo");
            ImGui::SameLine();
            ImGui::Text("%s", albedo.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##2"))
            {
                showFileDialog = true;
                showOpenButton = true;
                fileTypeOperation = FileTypeOperation::albedoTexture;
            }

            ImGui::Text("Normal");
            ImGui::SameLine();
            ImGui::Text("%s", normal.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##3"))
            {
                showFileDialog = true;
                showOpenButton = true;
                fileTypeOperation = FileTypeOperation::normalTexture;
            }

            ImGui::Text("Metalness");
            ImGui::SameLine();
            ImGui::Text("%s", metalness.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##5"))
            {
                showFileDialog = true;
                showOpenButton = true;
                fileTypeOperation = FileTypeOperation::metalnessTexture;
            }

            ImGui::Text("Roughness");
            ImGui::SameLine();
            ImGui::Text("%s", roughness.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##6"))
            {
                showFileDialog = true;
                showOpenButton = true;
                fileTypeOperation = FileTypeOperation::roughnessTexture;
            }

            ImGui::Text("AO");
            ImGui::SameLine();
            ImGui::Text("%s", ao.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##7"))
            {
                showFileDialog = true;
                showOpenButton = true;
                fileTypeOperation = FileTypeOperation::aoTexture;
            }

            ImGui::End();
        }
    }

    void updateAnimator(float deltatime)
    {
        if(selectedAnimationIndex != -1 && character != nullptr && animations.size() != 0)
        character->animator->UpdateAnimation(deltatime);
    }

    void updateFinalBoneMatrix(Shader ourShader)
    {
        shaderOfSelectedModel = &ourShader;
        if(character->animator != nullptr)
        {
            auto transforms = character->animator->GetFinalBoneMatrices();
            for (int i = 0; i < transforms.size(); ++i)
                ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }
        else{
            //MAXBONES 100
                for (int i = 0; i < 100; ++i)
                ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", 1.0f);
        }
        if(character->model != nullptr)
        {
            shaderOfSelectedModel->setInt("displayBoneIndex", selectedBoneId);
        }
    }

private:
    float m = 0;
    std::vector<Model *> *mModels;
    int selectedModelIndex = -1;
    int coordinateSpace = -1;
    bool isFirstFrame = true;

    bool showFileDialog=false;
    bool loadModelWindow=false;
    bool showOpenButton=false;

    std::string fileExtension;
    std::string filePath;

    FileTypeOperation fileTypeOperation;

    Character* character = NULL;
    Shader* shaderOfSelectedModel;
    int selectedBoneId = 0;

    std::string modelfileName="";
    std::string albedo="";
    std::string normal="";
    std::string metalness="";
    std::string roughness="";
    std::string ao="";
    std::string saveAsFileName = "";

    std::string error="";

    std::vector<Animation*> animations;
    std::vector<const char*> animationNames;
    int selectedAnimationIndex = -1;

    std::string errorMessage = ""; //Duplicate error declaration on lie 588 and 564.
    // Maybe we can have these at a common place to we can handle them better 

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
