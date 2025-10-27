#include <UI/Shared/ComboUI.hpp>
#include <EngineState.hpp>
#include <imgui.h>

bool UI::Shared::comboUI(std::string label, int &selectedIndex, std::map<std::string, std::string> filesMap)
{
    std::vector<std::string> smNames;
    std::vector<std::string> smKeys;
    smNames.push_back("None");
    for (auto& file : filesMap) {
        smNames.push_back(file.second);
        smKeys.push_back(file.first);        
    }

    // Build the const char* view array (pointers valid as long as smNames lives)
    std::vector<const char*> smNamePtrs;
    smNamePtrs.reserve(smNames.size());
    for (auto& s : smNames) smNamePtrs.push_back(s.c_str());

    auto& ui = *getUIState().characterUIState;
    
    return ImGui::Combo(
        label.c_str(),
        &selectedIndex,
        smNamePtrs.data(),
        (int)smNamePtrs.size());
}