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
    std::vector<Model *> *mModels;
    int selectedIndex;  // Index of the currently selected radio button
    bool isFirstFrame = true;
};
