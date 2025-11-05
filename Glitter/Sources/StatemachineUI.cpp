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

// Build dropdown options once per draw
std::vector<std::string> stateNames;
stateNames.reserve(statemachine->states.size());
for (auto* s : statemachine->states) stateNames.push_back(s->stateName);

// pointers valid as long as stateNames lives
std::vector<const char*> stateNamePtrs;
stateNamePtrs.reserve(stateNames.size());
for (auto& s : stateNames) stateNamePtrs.push_back(s.c_str());

// --- UI ---
for (auto& i : smUI->values)
{
   ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
   std::string child_id = "Card##" + i.statename;

   if (ImGui::BeginChild(child_id.c_str(), ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY)) {
   ImGui::TextColored(ImVec4(1,1,1,1), "%s", i.statename.c_str());
   ImGui::Separator();

    ImGui::PushID(i.statename.c_str());

    // A tidy framed card with a header
    if (ImGui::CollapsingHeader(i.statename.c_str(),
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_FramePadding |
        ImGuiTreeNodeFlags_SpanAvailWidth))
    {
        // Optional subtle spacing
        ImGui::Spacing();

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
                    int currentIndex = std::clamp(cond.IndexToState, 0, (int)stateNamePtrs.size() - 1);

                    // Label left, combo right (same row)
                    ImGui::TextUnformatted("To State");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(-1.0f); // fill column
                    if (ImGui::Combo("##ToStateCombo",
                                     &currentIndex,
                                     stateNamePtrs.data(),
                                     (int)stateNamePtrs.size()))
                    {
                        cond.IndexToState = currentIndex;
                    }
                }

                // --- Right column: Condition multiline ---
                ImGui::TableSetColumnIndex(1);
                {
                    ImGui::TextUnformatted("Condition");
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

    ImGui::PopID();

   }
ImGui::EndChild();
ImGui::PopStyleVar();
ImGui::Spacing();
}


   ImGui::End();
}