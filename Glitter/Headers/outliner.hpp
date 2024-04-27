#include "imgui.h"
#include <vector>
#include "model.hpp"

class Outliner {
public:
    // Constructor
    Outliner(std::vector<Model *> *models) : mModels(models), selectedIndex(-1) {
        // Initialize the items array or any other setup needed
    }

    // Render the radio buttons
    void Render() {
        ImGui::SetNextWindowSize(ImVec2(200, 300));
        ImGui::Begin("Outliner");
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

        //Show transformation of the selected model
        ImGui::Text("Translation");
        ImGui::PushItemWidth(40);
        if(selectedIndex > -1)
        {
            ImGui::DragFloat("X", &(*mModels)[selectedIndex]->model[3][0], 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", 0);
            ImGui::SameLine();
            ImGui::DragFloat("Y", &(*mModels)[selectedIndex]->model[3][1], 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", 0);
            ImGui::SameLine();
            ImGui::DragFloat("Z", &(*mModels)[selectedIndex]->model[3][2], 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", 0);
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

private:
    float m = 0;
    std::vector<Model *> *mModels;
    int selectedIndex = -1;  // Index of the currently selected radio button
    bool isFirstFrame = true;
};
