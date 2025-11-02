#include <UI/StatemachineUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>
#include <UI/Shared/InputText.hpp>

void UI::StatemachineUI::populateDelegateNodes(Controls::StateMachine* statemachine)
{
   auto states = statemachine->states;
   for (auto i = 0; i<  states.size(); i++)
   {

      // Go through each of states again and find which of them point to current index in this loop. 
      // That is how many inputs we have for this node.
      auto t = new GraphEditor::Template();
      ImU8 inputCount = 0;
      ImU8 outputCount = states[i]->toStateWhenCondition.size();

      for(auto j = 0; j<  states.size(); j++)
      {
         for (auto k = 0; k<  states[j]->toStateWhenCondition.size(); k++)
         {
            if( i == k)
            {
               inputCount++;
               delegate.mLinks.push_back(
                  {
                     (GraphEditor::NodeIndex)k,
                     0,
                     (GraphEditor::NodeIndex)i,
                     0,
                  }
               );
            }
         }
         
      }

      delegate.mTemplates.push_back(
         {
            IM_COL32(160, 160, 180, 255),
            IM_COL32(100, 100, 140, 255),
            IM_COL32(110, 110, 150, 255),
            inputCount,
            nullptr,
            nullptr,
            outputCount,
            nullptr,
            nullptr
         }
      );
      
      delegate.mNodes.push_back(
         {
            states[i]->stateName.c_str(),
            (GraphEditor::TemplateIndex)i,
            i*100.0f, i*100.0f,
            false
         }
      );
   }
   
}

void UI::StatemachineUI::draw(Controls::StateMachine* statemachine, bool &showUI)
{
   auto smUI = getUIState().statemachineUIState;

   smUI->delegate.clear();

   // Read value from selected statemachine to show in UI.
   auto state = statemachine->getStateGraph();
   smUI->populateDelegateNodes(statemachine); // need to process like an array otherwise will endup in infinite loop.


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