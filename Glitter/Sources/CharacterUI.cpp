#include <UI/CharacterUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>

void UI::CharacterUI::draw(Character* character, bool &showUI)
{
    std::vector<std::string> smNames;
    std::vector<std::string> smKeys;
    smNames.push_back("None");
    for (auto& file : EngineState::state->engineRegistry->statemachineFileMap) {
        smNames.push_back(file.second);
        smKeys.push_back(file.first);        
    }

    // Build the const char* view array (pointers valid as long as smNames lives)
    std::vector<const char*> smNamePtrs;
    smNamePtrs.reserve(smNames.size());
    for (auto& s : smNames) smNamePtrs.push_back(s.c_str());

    // UI
    if (ImGui::Begin("Character", &showUI)) 
    {
        auto& ui = *getUIState().characterUIState;
        if (ImGui::Combo("Select Statemachine",
                            &ui.selectedStateMachineIndex,
                            smNamePtrs.data(),
                            (int)smNamePtrs.size())) 
        {
            if(ui.selectedStateMachineIndex == 0){
                // None selected so delete state machine instance on the character;
                character->deleteStateMachine();
            }

            if (ui.selectedStateMachineIndex > 0 &&
                ui.selectedStateMachineIndex <= (int)EngineState::state->statemachines.size()) 
            {
                auto guid = smKeys[ui.selectedStateMachineIndex -1];
                character->loadStateMachine(guid);
            }
        }
    }
    ImGui::End();
}