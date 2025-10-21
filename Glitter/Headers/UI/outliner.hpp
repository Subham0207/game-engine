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
#include <Renderable/renderable.hpp>

#include "assimp/scene.h"
#include "Character/Character.hpp"
#include "Level/Level.hpp"
#include <filesystem>
#include <UI/Blendspace2DUI.hpp>
namespace fs = std::filesystem;


class Outliner 
{
public:
    // Constructor
    Outliner(std::vector<Renderable *> renderable) {
        // Initialize the items array or any other setup needed
        getUIState().renderables = *EngineState::state->activeLevel->renderables;
        getUIState().selectedRenderableIndex = -1;
    }

    // Render the radio buttons
    void Render(Level &lvl);

    // Get the index of the currently selected radio button
    int GetSelectedIndex() const {
        return getUIState().selectedRenderableIndex;
    }
    void setSelectedIndex(int newSelectedIndex){
    getUIState().selectedRenderableIndex = newSelectedIndex;
    }

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
