#include <UI/CharacterUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>

void UI::CharacterUI::draw(Character* character, bool &showUI)
{
    std::vector<std::string> smNames;
    smNames.push_back("None");
    smNames.reserve(EngineState::state->statemachines.size());
    for (auto& sm : EngineState::state->statemachines) {
    smNames.push_back(sm->contentName());  // strings own their storage
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
            if(ui.selectedStateMachineIndex == 0){}
            // None selected so delete state machine instance on the character;

            if (ui.selectedStateMachineIndex > 0 &&
                ui.selectedStateMachineIndex <= (int)EngineState::state->statemachines.size()) 
            {
                auto sm = EngineState::state->statemachines[ui.selectedStateMachineIndex -1]; // subtract 1 since [None, statemachine1, ...]
                character->loadStateMachine(sm->getGUID());
            }
        }
    }
    ImGui::End();
}