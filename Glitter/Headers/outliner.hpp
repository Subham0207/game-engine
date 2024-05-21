#include "imgui.h"
#include <vector>
#include "model.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include "state.hpp"


class Outliner {
public:
    // Constructor
    Outliner(std::vector<Model *> *models) : mModels(models), selectedIndex(-1) {
        // Initialize the items array or any other setup needed
    }

    // Render the radio buttons
    void Render() {
        ImGui::SetNextWindowSize(ImVec2(500, 300));
        ImGui::Begin("Outliner");

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
            if (ImGui::RadioButton( name.c_str(), &selectedIndex, i)) {
            }
            // Optionally, pop style changes here if you made any
        }

        ImGui::NewLine();

        //Show transformation of the selected model
        ImGui::PushItemWidth(40);
        if(selectedIndex > -1)
        {
            glm::mat4& modelMatrix = (*mModels)[selectedIndex]->model;
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

        if(isFirstFrame){
        ImGui::SetWindowFocus(false);
        isFirstFrame = false;
        }
    }

    // Get the index of the currently selected radio button
    int GetSelectedIndex() const {
        return selectedIndex;
    }
    void setSelectedIndex(int newSelectedIndex){
       selectedIndex = newSelectedIndex;
    }

    void applyRotation(glm::mat4& modelMatrix, glm::vec3 rotationDegrees, bool isLocal) {
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


private:
    float m = 0;
    std::vector<Model *> *mModels;
    int selectedIndex = -1;  // Index of the currently selected radio button
    int coordinateSpace = -1;
    bool isFirstFrame = true;
};
