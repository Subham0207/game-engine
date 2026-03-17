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
#include "Components/StateMachineTransition.hpp"
#include "NodeGraphEditorSpace.hpp"
#include "NodeGraphRenderContext.hpp"
#include "NodeGraphViewRegistry.hpp"

class NodeGraph
{
    public:
        NodeGraph();
        void drawUI();

    private:
        std::vector<NodeGraphNode> nodes;
        std::vector<CommentBox> comments;

		// State-machine component models.
		std::vector<StateMachineNode> stateNodes;
		std::vector<StateMachineTransition> stateTransitions;

        // Per-frame cached editor space mapping
        NodeGraphEditorSpace editorSpace;

        // Layered UI system
        NodeGraphViewRegistry views;
  NodeGraphRenderContext renderCtx{editorSpace,
    false, false, false, false, false, ImVec2(0.0f, 0.0f), NodeGraphInteractionState{},
    nodes, comments,
    stateNodes, stateTransitions};

};


#endif //GLITTER_NODEGRAPH_HPP