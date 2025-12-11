#include <UI/AI_UI.hpp>
#include <imgui.h>
#include <EngineState.hpp>
#include <UI/Shared/ComboUI.hpp>

namespace UI
{
    AI_UI::AI_UI()
    {
        filename = "AI";
        showUI = false;
    }

    void AI_UI::start()
    {
        getUIState().ai_ui_state->showUI = true;

        //List all the character instance ids in the level.
        for(auto itr: getActiveLevel().instanceIdToSerializableMap)
        {
            if(auto character = dynamic_cast<Character*>(itr.second))
            {
                getUIState().ai_ui_state->charactersList.push_back(character->getName());
            }
        }
    }

    void AI_UI::draw()
    {
        if(ImGui::Begin(filename.c_str(), &showUI))
        {
            if(UI::Shared::comboUI(
                "Select a character from level",
                selectedCharacterFromList,
                charactersList
            ))
            {
                
            }
        }
        ImGui::End();
    }
}