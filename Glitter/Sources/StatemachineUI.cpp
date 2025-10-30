#include <UI/StatemachineUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>

void UI::StatemachineUI::draw(bool &showUI)
{
   auto smUI = getUIState().statemachineUIState;
    ImGui::Begin("State Machine", &showUI);
         if (ImGui::Button("Fit all nodes"))
         {
            smUI->fit = GraphEditor::Fit_AllNodes;
         }
         ImGui::SameLine();
         if (ImGui::Button("Fit selected nodes"))
         {
            smUI->fit = GraphEditor::Fit_SelectedNodes;
         }
         GraphEditor::Show(
            smUI->delegate,
            smUI->options,
            smUI->viewState,
            true,
            &smUI->fit);
    ImGui::End();
}