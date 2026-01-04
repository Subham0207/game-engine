#include <UI/AI_UI.hpp>
#include <imgui.h>
#include <EngineState.hpp>
#include <UI/Shared/ComboUI.hpp>

#include "AI/AI.hpp"
#include "UI/Shared/EditableText.hpp"

namespace UI
{
    AI_UI::AI_UI()
    {
        filename= {
            "AI",
            "AI",
            false
         };
        showUI = false;
        selectedCharacterFromList = 0; // NONE is 0
    }

    void AI_UI::start()
    {
        getUIState().ai_ui_state->showUI = true;

        //List all the character instance ids in the level.
        for(auto itr: getActiveLevel().instanceIdToSerializableMap)
        {
            if(auto character = std::dynamic_pointer_cast<Character>(itr.second))
            {
                getUIState().ai_ui_state->charactersList.push_back(character->getName());
                getUIState().ai_ui_state->characters.push_back(character);
            }
        }
    }

    void AI_UI::draw()
    {
        if(ImGui::Begin(filename.value.c_str(), &showUI))
        {
            Shared::EditableTextUI("##savethistatemachine", filename);
            if(Shared::comboUI(
                "Select a character from level",
                selectedCharacterFromList,
                charactersList
            ))
            {
                
            }
            if (ImGui::Button("Save"))
            {
                save();
            }
        }
        ImGui::End();
    }

    void AI_UI::save() const
    {
        //Create a AI class object. Pass all the variables to it and call save no it.
        const auto character = characters[selectedCharacterFromList-1]; // -1 to account for 1 none.
        auto ai = new AI::AI(character,filename.value);
        auto dir = fs::path(EngineState::state->currentActiveProjectDirectory) / "Assets";
        ai->save(dir);
    }
}
