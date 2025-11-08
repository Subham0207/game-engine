#include <UI/StatemachineUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>
#include <UI/Shared/InputText.hpp>
#include <string>
#include <EngineState.hpp>
#include <map>
#include <algorithm>
#include <iterator>

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
      auto blendspaces = &EngineState::state->engineRegistry->blendpaceFileMap;
      auto animations = &EngineState::state->engineRegistry->animationsFileMap;
      smUI->blendspaces.blendspaceguids.push_back("None");
      smUI->blendspaces.blendspacenames.push_back("None");
      smUI->animations.animationguids.push_back("None");
      smUI->animations.animationnames.push_back("None");

      for (const auto& kv : *blendspaces) {
         smUI->blendspaces.blendspaceguids.push_back(kv.first.c_str());
         smUI->blendspaces.blendspacenames.push_back(kv.second.c_str());
      }

      for (const auto& kv : *animations) {
         smUI->animations.animationguids.push_back(kv.first);
         smUI->animations.animationnames.push_back(kv.second.c_str());
      }
      
      for(auto &&i: statemachine->states)
      {
         auto stateUI = StateUI();
         stateUI.statename = i->stateName;
         stateUI.toStateWhenCondition = std::vector<ToStateWhenConditionUI>();

         auto guids = &smUI->animations.animationguids;
         auto bguids = &smUI->blendspaces.blendspaceguids;

         stateUI.animationIndex = i->animationGuid != "" ? std::distance(guids->begin(), std::find(guids->begin(), guids->end(), i->animationGuid)): 0;
         stateUI.blendspaceIndex = i->blendspaceGuid != "" ? std::distance(bguids->begin(), std::find(bguids->begin(), bguids->end(), i->blendspaceGuid)): 0;

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

      smUI->stateNamePtrs.reserve(statemachine->states.size());
      for (auto* s : statemachine->states) smUI->stateNamePtrs.push_back(s->stateName.c_str());

   }


   ImGui::Begin("State Machine", &showUI);

      UI::Shared::InputText("##statemachine", smUI->temporaryNameForSave);
      if(ImGui::Button("Save"))
      {
         auto loc = fs::path(EngineState::state->currentActiveProjectDirectory) / "Assets";
         statemachine->setFileName(smUI->temporaryNameForSave);
         statemachine->save(loc);
      }

// --- UI ---
if(smUI->values.size() > 0)
for (auto& i : smUI->values)
{
   ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));
   ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
   ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.45f, 0.45f, 0.50f, 1.0f));

   const float margin = 12.0f; // adjust to taste

   // Top margin
   ImGui::Dummy(ImVec2(0, margin));

   std::string child_id = "Card##" + i.statename;

   if (ImGui::BeginChild(child_id.c_str(), ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY)) {

    // A tidy framed card with a header
    if (ImGui::CollapsingHeader(i.statename.c_str(),
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_FramePadding |
        ImGuiTreeNodeFlags_SpanAvailWidth))
    {
        // Optional subtle spacing
        ImGui::Spacing();

      ImGui::Text("Blendspace: ");
      ImGui::SameLine();
      if (ImGui::Combo("##Blendspace",
                        &i.blendspaceIndex,
                        smUI->blendspaces.blendspacenames.data(),
                        (int)smUI->blendspaces.blendspacenames.size()))
      {}

      ImGui::Text("Animation: ");
      ImGui::SameLine();
      if (ImGui::Combo("##Animation",
                        &i.animationIndex,
                        smUI->animations.animationnames.data(),
                        (int)smUI->animations.animationnames.size()))
      {}

        // 2-column layout: To-State (Combo) | Condition (Multiline)
        if (ImGui::BeginTable("TransitionsTable", 2,
                ImGuiTableFlags_SizingStretchProp |
                ImGuiTableFlags_Resizable |
                ImGuiTableFlags_BordersInnerV |
                ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("To State", ImGuiTableColumnFlags_WidthStretch, 0.35f);
            ImGui::TableSetupColumn("Condition", ImGuiTableColumnFlags_WidthStretch, 0.65f);
            ImGui::TableHeadersRow();

            for (size_t j = 0; j < i.toStateWhenCondition.size(); ++j)
            {
                auto& cond = i.toStateWhenCondition[j];
                ImGui::TableNextRow();

                // --- Left column: To-State dropdown ---
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(static_cast<int>(j));
                {
                    // Clamp to valid range for safety
                    int currentIndex = std::clamp(cond.IndexToState, 0, (int)smUI->stateNamePtrs.size() - 1);

                    // Label left, combo right (same row)
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(-1.0f); // fill column
                    if (ImGui::Combo("##ToStateCombo",
                                     &currentIndex,
                                     smUI->stateNamePtrs.data(),
                                     (int)smUI->stateNamePtrs.size()))
                    {
                        cond.IndexToState = currentIndex;
                    }
                }

                // --- Right column: Condition multiline ---
                ImGui::TableSetColumnIndex(1);
                {
                    ImGui::SetNextItemWidth(-1.0f);
                    // Tall enough for a few lines, grows with column width
                    ImGui::InputTextMultiline("##Condition",
                        cond.WhenCondition.data(),
                        cond.WhenCondition.size(),
                        ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4));
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        ImGui::Spacing();
    }

   }
ImGui::EndChild();


ImGui::PopStyleVar(2);  
ImGui::PopStyleColor(2);
ImGui::Separator();
ImGui::Spacing();

ImGui::Dummy(ImVec2(0, margin));
}


   ImGui::End();
}

Controls::StateMachine* UI::StatemachineUI::start()
{
   auto statemachine = new Controls::StateMachine();
   getUIState().statemachineUIState->UIOpenedForStatemachine = statemachine;
   getUIState().statemachineUIState->showStateMachineUI = true;
   getUIState().statemachineUIState->firstFrame = true;
   getUIState().statemachineUIState->values.clear();
   return statemachine;
}

void UI::StatemachineUI::save(Controls::StateMachine* statemachine)
{
   for (size_t i = 0; i < values.size(); i++)
   {
      //Set statename
      statemachine->states[i]->stateName = values[i].statename;

      auto fromlist = &values[i].toStateWhenCondition;
      auto toList = &statemachine->states[i]->toStateWhenCondition;

      for (size_t j = 0; j < fromlist->size(); j++)
      {
         //set index of toState
         toList->at(j).index = fromlist->at(j).IndexToState;
         //set condition for toState
         toList->at(j).condition.setSource(std::string(fromlist->at(j).WhenCondition.data(), MAX_SOURCE_LENGTH));
      }
   }
     
}