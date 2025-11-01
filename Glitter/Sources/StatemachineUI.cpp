#include <UI/StatemachineUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>
#include <UI/Shared/InputText.hpp>

void UI::StatemachineUI::populateDelegateNodes(Controls::StateMachine* statemachine, std::shared_ptr<Controls::State> currentState)
{
   if(!currentState)
   return;

   auto inputNamePointers = new std::vector<const char*>();
   auto outputNamePointers = new std::vector<const char*>();

   for (auto &&s : currentState->toStateWhenCondition)
   {
      outputNamePointers->push_back(s.state->stateName.c_str());
      populateDelegateNodes(statemachine, s.state);
   }
   ImU8 outputLinksCount = currentState->toStateWhenCondition.size();
   delegate.mTemplates.push_back(
      {
         IM_COL32(160, 160, 180, 255),
         IM_COL32(100, 100, 140, 255),
         IM_COL32(110, 110, 150, 255),
         1,
         inputNamePointers->data(),
         nullptr,
         outputLinksCount,
         outputNamePointers->data(),
         nullptr
      }       
   );
   
}

void UI::StatemachineUI::draw(Controls::StateMachine* statemachine, bool &showUI)
{
   auto smUI = getUIState().statemachineUIState;

   smUI->delegate.clear();

   // Read value from selected statemachine to show in UI.
   auto state = statemachine->getStateGraph();
   smUI->populateDelegateNodes(statemachine, state); // need to process like an array otherwise will endup in infinite loop.


    ImGui::Begin("State Machine", &showUI);

         UI::Shared::InputText("##NameBlendspace", smUI->temporaryNameForSave);
         if(ImGui::Button("Save"))
         {
            auto loc = fs::path(EngineState::state->currentActiveProjectDirectory) / "Assets";
            statemachine->setFileName(smUI->temporaryNameForSave);
            statemachine->save(loc);
         }

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