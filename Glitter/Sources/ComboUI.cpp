#include <UI/Shared/ComboUI.hpp>
#include <EngineState.hpp>
#include <imgui.h>

bool UI::Shared::comboUI(const char* label, int &selectedIndex, std::vector<std::string> &options)
{
    // Build the const char* view array (pointers valid as long as smNames lives)
    std::vector<const char*> smNamePtrs;
    smNamePtrs.reserve(options.size()+1);
    smNamePtrs.push_back("None");
    for (auto& s : options) smNamePtrs.push_back(s.c_str());
    
    return ImGui::Combo(
        label,
        &selectedIndex,
        smNamePtrs.data(),
        (int)smNamePtrs.size());
}