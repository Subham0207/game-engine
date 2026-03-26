//
// Created by subha on 26-03-2026.
//

#include <NodeGraph/Views/StateMachineView.hpp>
#include <NodeGraph/FlowScript/FlowScript.hpp>

void StateMachineView::EditCondition(const StateMachineLink* selectedLink) const
{
    if (ImGui::Button("Edit Condition"))
    {
        if (flowscriptRef && !selectedLink->condition.empty())
            flowscriptRef->setCompiledLua(selectedLink->condition);
    }
}
