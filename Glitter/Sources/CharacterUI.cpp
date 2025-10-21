#include <UI/CharacterUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>

void UI::CharacterUI::draw(Character* character, bool &showUI)
{
    std::vector<const char*> statemachineNames;
    for (const auto& sm : EngineState::state->statemachines) {
        statemachineNames.push_back(sm->contentName().c_str());
    }

    if(ImGui::Begin("Character", &showUI))
    {
        if (ImGui::Combo("Select Statemachine",
        &getUIState().characterUIState->selectedStateMachineIndex,
        statemachineNames.data(),
        statemachineNames.size())) {
            auto sm = EngineState::state->statemachines[getUIState().characterUIState->selectedStateMachineIndex];
            character->loadStateMachine(sm->getGUID());
        }
        ImGui::End();
    }
}