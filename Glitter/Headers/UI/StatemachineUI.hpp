#pragma once
#include <vector>
#include <UI/Types/StateMachineLink.hpp>
#include <UI/Types/StateMachineNode.hpp>
#include <UI/SMGraphEditor.hpp>
#include <GraphEditor.h>
#include <Controls/statemachine.hpp>

namespace UI
{
    class StatemachineUI
    {
        public:
            StatemachineUI(){
                UIOpenedForStatemachine = nullptr;
                showStateMachineUI = false;
                delegate = UI::StateMachineGraph::GraphEditorDelegate();
                firstFrame = true;
                
                options.mDisplayLinksAsCurves = false;
            }

            static void draw(Controls::StateMachine* statemachine, bool &showUI);

            bool showStateMachineUI;
            Controls::StateMachine* UIOpenedForStatemachine;

            bool firstFrame;
        private:

            void populateDelegateNodes(Controls::StateMachine* statemachine);

            std::string temporaryNameForSave;

            GraphEditor::Options options;
            UI::StateMachineGraph::GraphEditorDelegate delegate;
            GraphEditor::ViewState viewState;
            GraphEditor::FitOnScreen fit = GraphEditor::Fit_None;
            bool showGraphEditor = true;

    };
}