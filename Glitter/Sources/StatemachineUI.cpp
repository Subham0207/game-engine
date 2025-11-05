#include <UI/StatemachineUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>
#include <UI/Shared/InputText.hpp>

UI::StatemachineUI::StatemachineUI(){
      UIOpenedForStatemachine = nullptr;
      showStateMachineUI = false;
      delegate = UI::StateMachineGraph::GraphEditorDelegate();
      firstFrame = true;
      
      options.mDisplayLinksAsCurves = false;

      values = std::vector<StateUI>();
}

void UI::StatemachineUI::draw(Controls::StateMachine* statemachine, bool &showUI)
{

   auto smUI = getUIState().statemachineUIState;

   if(smUI->firstFrame)
   {
      for(auto &&i: statemachine->states)
      {
         auto stateUI = StateUI();
         stateUI.statename = i->stateName;
         stateUI.toStateWhenCondition = std::vector<ToStateWhenConditionUI>();

         for(auto j = 0; j<i->toStateWhenCondition.size(); j++)
         {
            ToStateWhenConditionUI toStateWhenCondition;
            toStateWhenCondition.IndexToState = i->toStateWhenCondition[j].index;

            auto condition = i->toStateWhenCondition[j].condition.source();
            const std::size_t n = std::min(condition.size(), MAX_SOURCE_LENGTH - 1);
            std::memcpy(toStateWhenCondition.WhenCondition.data(), condition.data(), n);
            
            stateUI.toStateWhenCondition.push_back(
               toStateWhenCondition
            );

         }
         smUI->values.push_back(stateUI);
      }

      smUI->firstFrame = false;
   }


   ImGui::Begin("State Machine", &showUI);

      UI::Shared::InputText("##statemachine", smUI->temporaryNameForSave);
      if(ImGui::Button("Save"))
      {
         auto loc = fs::path(EngineState::state->currentActiveProjectDirectory) / "Assets";
         statemachine->setFileName(smUI->temporaryNameForSave);
         statemachine->save(loc);
      }

      for(auto &&i: smUI->values)
      {
         // Nodes
         ImGui::Text("statename %s", i.statename.c_str());
         for(auto j = 0; j<i.toStateWhenCondition.size(); j++)
         {
            // drop down to select to node from a list.
            
            auto toStateIndex = i.toStateWhenCondition[j].IndexToState;
            ImGui::Text("To: %d", toStateIndex);

            std::string condId = "##Condition_" + i.statename + "_" + std::to_string(toStateIndex);
            ImGui::Text("Condition: ");
            ImGui::InputTextMultiline(
               condId.c_str(),
               i.toStateWhenCondition[j].WhenCondition.data(),
               UI::MAX_SOURCE_LENGTH,
               ImVec2(0, 0)
            );
         }
      }

   ImGui::End();
}