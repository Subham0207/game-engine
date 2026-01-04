#include <UI/CharacterUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>

UI::CharacterUI::CharacterUI()
{
    showCharacterUI = false;
    characterName = "Character UI";
}
void UI::CharacterUI::draw(bool &showUI)
{
    if (ImGui::Begin(characterName.c_str()))
    {

    }
    ImGui::End();
}