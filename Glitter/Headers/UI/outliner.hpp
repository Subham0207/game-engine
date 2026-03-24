#pragma once
#include <vector>
#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "EngineState.hpp"
#include <Renderable/renderable.hpp>
#include "Level/Level.hpp"
namespace fs = std::filesystem;


class Outliner 
{
public:
    // Constructor
    Outliner() {
        // Initialize the items array or any other setup needed
        getUIState().selectedRenderableIndex = -1;
    }

    // Render the outliner. The editor camera move speed is provided by the owning window
    // (e.g. EditorWindow) so Outliner doesn't need to reach into EngineState for per-window settings.
    void Render(Level &lvl, float& editorCameraMoveSpeed);

    // Optional: allow the owning editor window to provide a request sink for opening tool windows.
    // This is intentionally a raw pointer: Outliner doesn't own it; EditorWindow does.
    struct WindowRequests
    {
        bool* openStateMachineWindow = nullptr;
    };

    WindowRequests windowRequests;

    // Get the index of the currently selected radio button
    int GetSelectedIndex() const {
        return getUIState().selectedRenderableIndex;
    }
    void setSelectedIndex(int newSelectedIndex){
    getUIState().selectedRenderableIndex = newSelectedIndex;
    }

private:
    bool scaleUniformly = false;

    void ModelAndTextureSelectionWindow();
    void ModelMatrixComponent();
    void levelControlsComponent(Level &lvl, float& editorCameraMoveSpeed);
    void modelSelectorComponent();
    void coordinateSystemSelectorComponent();
    void manageModels();
    void manageAnimationsForSelectedModel();
    void popupForErrorsAndWarning();
    void debugOptions();
    void handlerForUIComponentsvisibility();
    void propertiesPanel();

    void applyRotation(glm::mat4& modelMatrix, glm::vec3 rotationDegrees, bool isLocal);
};
