#include <UIState/UIState.hpp>
#include <UI/CharacterUI.hpp>
#include <UI/StatemachineUI.hpp>

ProjectAsset::UIState::UIState()
{
    characterUIState = new UI::CharacterUI();
    blendspace2DUIState = new UI::Blendspace2DUI();
    statemachineUIState = new UI::StatemachineUI();
}