#pragma once
#include "imgui.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"
#include "EngineState.hpp"
#include "3DModel/Animation/Animator.hpp"
#include  <UI/FileExplorer.hpp>

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
            getUIState().selectAFile = true;
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
            getUIState().selectAFile = true;
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
            getUIState().selectAFile = true;
            getUIState().showOpenButton = false;
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::saveModel;
        }
        if(ImGui::Button("Load a Model"))
        {
            getUIState().selectAFile = true;
            getUIState().showOpenButton = true;
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::loadModel;
        }
        if(ImGui::Button("Load animation for model"))
        {
            getUIState().selectAFile = true;
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
        

        if(getUIState().selectAFile)
        {
            ProjectAsset::selectOrLoadAFileFromFileExplorer(fs::current_path().string(), State::state->uiState.fileNames);
        }
        else if(getUIState().saveAFile){
            ProjectAsset::saveAFile(fs::current_path().string(), State::state->uiState.fileNames);
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

        ProjectAsset::selectOrLoadAFileFromFileExplorer(currentPath, fileNames);
        ProjectAsset::saveAFile(currentPath, fileNames);
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
                getUIState().selectAFile = true;
                getUIState().showOpenButton = true;
                getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::importModelFile;
            }

            ImGui::Text("Albedo");
            ImGui::SameLine();
            ImGui::Text("%s", getUIState().albedo.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##2"))
            {
                getUIState().selectAFile = true;
                getUIState().showOpenButton = true;
                getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::albedoTexture;
            }

            ImGui::Text("Normal");
            ImGui::SameLine();
            ImGui::Text("%s", getUIState().normal.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##3"))
            {
                getUIState().selectAFile = true;
                getUIState().showOpenButton = true;
                getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::normalTexture;
            }

            ImGui::Text("Metalness");
            ImGui::SameLine();
            ImGui::Text("%s", getUIState().metalness.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##5"))
            {
                getUIState().selectAFile = true;
                getUIState().showOpenButton = true;
                getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::metalnessTexture;
            }

            ImGui::Text("Roughness");
            ImGui::SameLine();
            ImGui::Text("%s", getUIState().roughness.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##6"))
            {
                getUIState().selectAFile = true;
                getUIState().showOpenButton = true;
                getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::roughnessTexture;
            }

            ImGui::Text("AO");
            ImGui::SameLine();
            ImGui::Text("%s", getUIState().ao.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##7"))
            {
                getUIState().selectAFile = true;
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
