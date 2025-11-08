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

    struct blendspaceUI
    {
        std::vector<std::string> blendspaceguids;
        std::vector<const char*> blendspacenames;
    };

    struct animationsUI
    {
        std::vector<std::string> animationguids;
        std::vector<const char*> animationnames;
    };

    struct ToStateWhenConditionUI{
        int IndexToState = -1;
        std::array<char, MAX_SOURCE_LENGTH> WhenCondition{};
    };

    struct StateUI{
        std::string statename;
        std::vector<ToStateWhenConditionUI> toStateWhenCondition;
        int animationIndex = 0;
        int blendspaceIndex = 0;
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
            animationsUI animations;
            blendspaceUI blendspaces;

            GraphEditor::Options options;
            UI::StateMachineGraph::GraphEditorDelegate delegate;
            GraphEditor::ViewState viewState;
            GraphEditor::FitOnScreen fit = GraphEditor::Fit_None;
            bool showGraphEditor = true;
            std::vector<StateUI> values;


    };
}