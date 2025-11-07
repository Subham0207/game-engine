#pragma once
#include <vector>
#include <UI/Types/StateMachineLink.hpp>
#include <UI/Types/StateMachineNode.hpp>
#include <UI/SMGraphEditor.hpp>
#include <GraphEditor.h>
#include <Controls/statemachine.hpp>

namespace UI
{
    constexpr std::size_t MAX_SOURCE_LENGTH = 256;

    struct ToStateWhenConditionUI{
        int IndexToState = -1;
        std::array<char, MAX_SOURCE_LENGTH> WhenCondition{};
    };

    struct StateUI{
        std::string statename;
        std::vector<ToStateWhenConditionUI> toStateWhenCondition;
    };

    class StatemachineUI
    {
        public:
            StatemachineUI();

            static void draw(Controls::StateMachine* statemachine, bool &showUI);
            static Controls::StateMachine* start();

            void save(Controls::StateMachine* statemachine);

            bool showStateMachineUI;
            Controls::StateMachine* UIOpenedForStatemachine;

            bool firstFrame;
        private:

            std::string temporaryNameForSave;
            std::vector<const char*> stateNamePtrs;

            GraphEditor::Options options;
            UI::StateMachineGraph::GraphEditorDelegate delegate;
            GraphEditor::ViewState viewState;
            GraphEditor::FitOnScreen fit = GraphEditor::Fit_None;
            bool showGraphEditor = true;
            std::vector<StateUI> values;


    };
}