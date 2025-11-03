#include <UI/StatemachineUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>
#include <UI/Shared/InputText.hpp>

void UI::StatemachineUI::draw(Controls::StateMachine* statemachine, bool &showUI)
{

   auto smUI = getUIState().statemachineUIState;

   if(smUI->firstFrame)
   {

   }


   ImGui::Begin("State Machine", &showUI);

      UI::Shared::InputText("##statemachine", smUI->temporaryNameForSave);
      if(ImGui::Button("Save"))
      {
         auto loc = fs::path(EngineState::state->currentActiveProjectDirectory) / "Assets";
         statemachine->setFileName(smUI->temporaryNameForSave);
         statemachine->save(loc);
      }

      for(auto &&i: statemachine->states)
      {
         ImGui::Text("statename %s", i->stateName.c_str());
         for(auto &&j: i->toStateWhenCondition)
         {
            ImGui::Text("To: %s", j.state->stateName.c_str());
            ImGui::Text("Condition: %s", j.condition.source().c_str());
         }
      }

   ImGui::End();
}