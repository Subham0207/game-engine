#include <UI/AI_UI.hpp>
#include <imgui.h>
#include <EngineState.hpp>

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
    }

    void AI_UI::draw()
    {
        if(ImGui::Begin(filename.c_str(), &showUI))
        {

        }
        ImGui::End();
    }
}