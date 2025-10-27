#include <UIState/UIState.hpp>
#include <UI/CharacterUI.hpp>

ProjectAsset::UIState::UIState()
{
    characterUIState = new UI::CharacterUI();
    blendspace2DUIState = new UI::Blendspace2DUI();
}