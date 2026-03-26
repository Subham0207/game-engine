//
// Created by subha on 26-03-2026.
//

#include <NodeGraph/Views/StateMachineView.hpp>
#include <NodeGraph/FlowScript/StatemachineFlowScript.hpp>

void StateMachineView::EditCondition(StateMachineLink* selectedLink) const
{
    if (ImGui::Button("Edit Condition"))
    {
        if (mSmflowscriptRef)
        {
            mSmflowscriptRef->setSelectedLink(selectedLink);
        }
    }
}
