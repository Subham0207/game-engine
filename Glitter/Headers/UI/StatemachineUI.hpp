#pragma once
#include <vector>
#include <UI/Types/StateMachineLink.hpp>
#include <UI/Types/StateMachineNode.hpp>
#include <UI/SMGraphEditor.hpp>
#include <GraphEditor.h>

namespace UI
{
    class StatemachineUI
    {
        public:
            StatemachineUI(){}

            static void draw(bool &showUI);

            bool showStateMachineUI;
        private:
            GraphEditor::Options options;
            UI::StateMachineGraph::GraphEditorDelegate delegate;
            GraphEditor::ViewState viewState;
            GraphEditor::FitOnScreen fit = GraphEditor::Fit_None;
            bool showGraphEditor = true;

    };
}