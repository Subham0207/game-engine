#include <UI/StatemachineUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>
#include <UI/Shared/InputText.hpp>
#include <string>
#include <EngineState.hpp>
#include <map>
#include <algorithm>
#include <iterator>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <Helpers/Trim.hpp>

UI::StatemachineUI::StatemachineUI(){
      UIOpenedForStatemachine = nullptr;
      showStateMachineUI = false;
      delegate = UI::StateMachineGraph::GraphEditorDelegate();
      firstFrame = true;
      
      options.mDisplayLinksAsCurves = false;

      values = std::vector<StateUI>();

      objToDelete = nullptr;

      entryStateIndex = 0;

      statemachinename = {
         "statemachine",
         "statemachine",
         false
      };
}

void UI::StatemachineUI::draw(Controls::StateMachine* statemachine, bool &showUI)
{

   auto smUI = getUIState().statemachineUIState;
   
   firstFrameHandler(statemachine);

   if(smUI->objToDelete)
   {
      smUI->values.erase(std::remove_if(smUI->values.begin(), smUI->values.end(),
         [&](const StateUI &x){return &x == smUI->objToDelete;}), smUI->values.end());
      smUI->objToDelete = nullptr;
   }


   ImGui::Begin("State Machine", &showUI);

      handlesave(smUI, statemachine);

      if (ImGui::Combo("##entryStateIndex",
      &smUI->entryStateIndex,
      smUI->stateNamePtrs.data(),
      (int)smUI->stateNamePtrs.size()))
      {
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

         std::string child_id = "Card##" + i.id;

         if (ImGui::BeginChild(child_id.c_str(), ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY)) 
         {
            if(UI::Shared::EditableTextUI(("##" + i.id).c_str(), i.statename))
            {
               smUI->reInitStateNamePtrs();
            }

            handleDelete(smUI, i);

            // A tidy framed card with a header
            if (ImGui::CollapsingHeader(i.statename.value.c_str(),
               ImGuiTreeNodeFlags_DefaultOpen |
               ImGuiTreeNodeFlags_FramePadding |
               ImGuiTreeNodeFlags_SpanAvailWidth))
            {
               // Optional subtle spacing
               ImGui::Spacing();

               drawStateVariables(smUI, i);

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
                        UI::StatemachineUI::drawToStateAndCondition(smUI, i, j);
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

      //Track new state changes here in this class members.
      // On save we can apply original class.
      if(ImGui::Button("Add State"))
      {
         smUI->newStateCounter +=1;
         auto statename = "new state" + std::to_string(smUI->newStateCounter);
         UI::Shared::EditableText text = {
            statename,
            statename,
            false
         };
         std::string guid = boost::uuids::to_string(boost::uuids::random_generator()());
         auto stateUI = UI::StateUI
         {
            guid,
            text,
            std::vector<ToStateWhenConditionUI>(),
            0,
            0
         };
         getUIState().statemachineUIState->values.push_back(stateUI);
         
         smUI->reInitStateNamePtrs();
      }

      if(ImGui::CollapsingHeader("Animations"))
      {
         for (int i = 0;i < smUI->animations.animationnames.size();i++)
         {
            ImGui::Text(smUI->animations.animationnames[i]);
            ImGui::SameLine();
            ImGui::Text(smUI->animations.animationguids[i].c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton(("Copy##"s + std::to_string(i)).c_str())) {
               ImGui::SetClipboardText(smUI->animations.animationguids[i].c_str());
            }
         }
      }

   ImGui::End();
}

Controls::StateMachine *UI::StatemachineUI::start()
{
   auto statemachine = new Controls::StateMachine();
   getUIState().statemachineUIState->UIOpenedForStatemachine = statemachine;
   getUIState().statemachineUIState->showStateMachineUI = true;
   getUIState().statemachineUIState->firstFrame = true;

   return statemachine;
}

void UI::StatemachineUI::handlesave(UI::StatemachineUI* smUI, Controls::StateMachine* statemachine)
{
   UI::Shared::EditableTextUI("##savethistatemachine", smUI->statemachinename);
   if(ImGui::Button("Save"))
   {
      auto loc = fs::path(EngineState::state->currentActiveProjectDirectory) / "Assets";
      if(statemachine->contentName() != "" && smUI->statemachinename.value != statemachine->contentName())
      {
         //This is essentially renaming a statemachine; we don't need to new guid since other parts of program might be using it
         //Delete existing file
         //And save this;
         statemachine->deleteFile();

         //we also need to re-initialize filesmap with correct filepath now + refresh any intances in memory (Life cycle of a serialzable)
      }
      statemachine->setFileName(smUI->statemachinename.value);
      auto statesMap = std::map<std::string, std::shared_ptr<Controls::State>>();
      for (auto i=0;i<smUI->values.size();i++)
      {
         auto state = std::make_shared<Controls::State>(smUI->values[i].statename.value);

         if(i == smUI->entryStateIndex)
         {
            statemachine->setActiveState(state);
         }

         auto animIndex = smUI->values[i].animationIndex;

         if(animIndex != 0)
         {
            // we just need to guid to load statemachine; Right now we only need to save it.
            state->animationGuid = smUI->animations.animationguids[animIndex];
         }
         auto blendspaceIndex = smUI->values[i].blendspaceIndex;
         if(blendspaceIndex != 0)
         {
            // we just need to guid to load statemachine; Right now we only need to save it.
            state->blendspaceGuid = smUI->blendspaces.blendspaceguids[blendspaceIndex];
         }

         statesMap[smUI->values[i].statename.value] = state;
      }

      for (auto &&i: smUI->values)
      {
         for (auto &&j : i.toStateWhenCondition)
         {
            auto toStateName = smUI->values[j.IndexToState].statename;
            auto toState = statesMap[toStateName.value];

            auto cleanedString = cleanChars(j.WhenCondition);
            
            statesMap[i.statename.value]->toStateWhenCondition.push_back(
               Controls::ToStateWhenCondition(
                  toState,
                  cleanedString
               )
            );
         }
         
      }

      //call statemachine save method.
      statemachine->save(loc);
      smUI->firstFrame = true;
   }
}

void UI::StatemachineUI::drawStateVariables(UI::StatemachineUI* smUI, UI::StateUI &state)
{
   ImGui::Text("Blendspace: ");
   ImGui::SameLine();
   if (ImGui::Combo("##Blendspace",
         &state.blendspaceIndex,
         smUI->blendspaces.blendspacenames.data(),
         (int)smUI->blendspaces.blendspacenames.size()))
   {}

   ImGui::Text("Animation: ");
   ImGui::SameLine();
   if (ImGui::Combo("##Animation",
         &state.animationIndex,
         smUI->animations.animationnames.data(),
         (int)smUI->animations.animationnames.size()))
   {}

   if(ImGui::Button("Add to State"))
   {
      state.toStateWhenCondition.push_back(
      ToStateWhenConditionUI()
      );
   }
}

void UI::StatemachineUI::handleDelete(UI::StatemachineUI* smUI, UI::StateUI &obj)
{
   if(ImGui::Button("Delete"))
   {
      smUI->objToDelete = &obj;
      smUI->reInitStateNamePtrs();
   }
}

void UI::StatemachineUI::drawToStateAndCondition(UI::StatemachineUI* smUI, UI::StateUI &stateUI, int id)
{
   auto& cond = stateUI.toStateWhenCondition[id];
   ImGui::TableNextRow();

   // --- Left column: To-State dropdown ---
   ImGui::TableSetColumnIndex(0);
   ImGui::PushID(static_cast<int>(id));
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

void UI::StatemachineUI::firstFrameHandler(Controls::StateMachine* statemachine)
{
   auto smUI = getUIState().statemachineUIState;

   
   if(smUI->firstFrame)
   {
      smUI->values.clear();
      smUI->stateNamePtrs.clear();
      smUI->blendspaces.blendspaceguids.clear();
      smUI->blendspaces.blendspacenames.clear();
      smUI->animations.animationguids.clear();
      smUI->animations.animationnames.clear();

      smUI->statemachinename = {
         statemachine->contentName(),
         statemachine->contentName(),
         false
      };
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
      
      auto index = 0;
      for(auto &&i: statemachine->states)
      {
         if(i->stateName == statemachine->getActiveState()->stateName)
         {
            smUI->entryStateIndex = index+1; //to account for None as a selection at 0th index.
         }

         auto stateUI = StateUI();
         stateUI.id =  boost::uuids::to_string(boost::uuids::random_generator()());
         stateUI.statename = {
            i->stateName,
            i->stateName,
            false
         };

         stateUI.toStateWhenCondition = std::vector<ToStateWhenConditionUI>();

         auto guids = &smUI->animations.animationguids;
         auto bguids = &smUI->blendspaces.blendspaceguids;

         stateUI.animationIndex = i->animationGuid != "" ? std::distance(guids->begin(), std::find(guids->begin(), guids->end(), i->animationGuid)): 0;
         stateUI.blendspaceIndex = i->blendspaceGuid != "" ? std::distance(bguids->begin(), std::find(bguids->begin(), bguids->end(), i->blendspaceGuid)): 0;

         for(auto j = 0; j<i->toStateWhenCondition.size(); j++)
         {
            ToStateWhenConditionUI toStateWhenCondition;
            toStateWhenCondition.IndexToState = i->toStateWhenCondition[j].index;

            auto condition = i->toStateWhenCondition[j].luaCondition.source();
            const std::size_t n = std::min(condition.size(), MAX_SOURCE_LENGTH - 1);
            std::memcpy(toStateWhenCondition.WhenCondition.data(), condition.data(), n);
            
            stateUI.toStateWhenCondition.push_back(
               toStateWhenCondition
            );

         }
         smUI->values.push_back(stateUI);

         index++;
      }

      smUI->firstFrame = false;

      smUI->stateNamePtrs.reserve(statemachine->states.size());
      for (auto* s : statemachine->states) smUI->stateNamePtrs.push_back(s->stateName.c_str());

   }
}

void UI::StatemachineUI::reInitStateNamePtrs()
{
   stateNamePtrs.clear();
   for (auto &&s : values) stateNamePtrs.push_back(s.statename.value.c_str());
}

void UI::StatemachineUI::save(Controls::StateMachine *statemachine)
{
     
}