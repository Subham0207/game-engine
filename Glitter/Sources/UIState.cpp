#include <UIState/UIState.hpp>
#include <UI/CharacterUI.hpp>
#include <UI/StatemachineUI.hpp>
#include <UI/PropertiesPanel.hpp>

ProjectAsset::UIState::UIState()
{
    characterUIState = new UI::CharacterUI();
    blendspace2DUIState = new UI::Blendspace2DUI();
    statemachineUIState = new UI::StatemachineUI();
    propretiesPanel = new UI::PropertiesPanel();
    ai = nullptr;
    renderNavMesh = false;
}