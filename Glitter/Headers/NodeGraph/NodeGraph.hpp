//
// Created by subha on 15-03-2026.
//

#ifndef GLITTER_NODEGRAPH_HPP
#define GLITTER_NODEGRAPH_HPP

#include <vector>
#include <string>
#include <imgui.h>

#include "Components/NodeGraphNode.hpp"

#include "Components/CommentBox.hpp"
#include "Components/StateMachineNode.hpp"
#include "Components/StateMachineLink.hpp"
#include "NodeGraphEditorSpace.hpp"
#include "NodeGraphRenderContext.hpp"
#include "NodeGraphViewRegistry.hpp"

class NodeGraph
{
    public:
        NodeGraph();
        void drawUI();
        // Draw inside an already-open ImGui region (e.g., child window / docked panel).
        // Unlike drawUI(), this does NOT call ImGui::Begin/End.
        void drawUIEmbedded();

    private:
        std::vector<NodeGraphNode> nodes;
        std::vector<CommentBox> comments;

		// State-machine component models.
		std::vector<StateMachineNode> stateNodes;
    std::vector<StateMachineLink> stateLinks;

        // Per-frame cached editor space mapping
        NodeGraphEditorSpace editorSpace;

        // Layered UI system
        NodeGraphViewRegistry views;
  NodeGraphRenderContext renderCtx{editorSpace,
    false, false, false, false, false, ImVec2(0.0f, 0.0f), NodeGraphInteractionState{},
    nodes, comments,
    stateNodes, stateLinks};

};


#endif //GLITTER_NODEGRAPH_HPP