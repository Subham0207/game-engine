#include <UIState/UIState.hpp>
#include <UI/CharacterUI.hpp>
#include <UI/StatemachineUI.hpp>
#include <UI/PropertiesPanel.hpp>
#include <UI/AI_UI.hpp>

ProjectAsset::UIState::UIState()
{
    characterUIState = new UI::CharacterUI();
    blendspace2DUIState = new UI::Blendspace2DUI();
    statemachineUIState = new UI::StatemachineUI();
    propretiesPanel = new UI::PropertiesPanel();
    ai = nullptr;
    renderNavMesh = false;
    ai_ui_state = new UI::AI_UI();
}