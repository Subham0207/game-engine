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
        getUIState().models = *State::state->activeLevel.models;
        getUIState().selectedModelIndex = -1;
    }

    // Render the radio buttons
    void Render(Level &lvl) {
        if(ImGui::Begin("Outliner"))
        {
            levelControlsComponent(lvl);

            coordinateSystemSelectorComponent();

            ImGui::NewLine();

            modelSelectorComponent();

            ImGui::NewLine();

            //Show transformation of the selected model
            ImGui::PushItemWidth(40);
            ModelMatrixComponent();

            manageModels();

            manageAnimationsForSelectedModel();

            debugOptions();

            popupForErrorsAndWarning();
        
            ImGui::End();
        }
        
        handlerForUIComponentsvisibility();

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



    //Handling Animation applying logic here: probably these should be called from character class idk right now we will see.
    void updateAnimator(float deltatime);

private:
    bool scaleUniformly = true;

    void ModelAndTextureSelectionWindow();
    void ModelMatrixComponent();
    void levelControlsComponent(Level &lvl);
    void modelSelectorComponent();
    void coordinateSystemSelectorComponent();
    void manageModels();
    void manageAnimationsForSelectedModel();
    void popupForErrorsAndWarning();
    void debugOptions();
    void handlerForUIComponentsvisibility();

    void applyRotation(glm::mat4& modelMatrix, glm::vec3 rotationDegrees, bool isLocal);
};
