//
// Created by subha on 15-03-2026.
//

#ifndef GLITTER_NODEGRAPH_HPP
#define GLITTER_NODEGRAPH_HPP

#include <vector>
#include <string>
#include <imgui.h>
#include <imnodes.h>

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
        virtual ~NodeGraph();
        void drawUI();
        // Draw inside an already-open ImGui region (e.g., child window / docked panel).
        // Unlike drawUI(), this does NOT call ImGui::Begin/End.
        void drawUIEmbedded();

        // Allow callers (or derived graphs) to inject additional UI elements in SCREEN space.
        // This is called once per frame inside the NodeGraph ImGui window, after the editor has drawn.
        // Keep it simple: just provide a callback that gets access to the current render context.
        using ScreenSpaceUiFn = void(*)(NodeGraphRenderContext& ctx, void* user);
        void setScreenSpaceUi(ScreenSpaceUiFn fn, void* user = nullptr)
        {
            screenSpaceUiFn = fn;
            screenSpaceUiUser = user;
        }

        // Minimal accessors for state-machine tooling.
        [[nodiscard]] const std::vector<StateMachineNode>& getStateNodes() const { return stateNodes; }
        [[nodiscard]] const std::vector<StateMachineLink>& getStateLinks() const { return stateLinks; }
        [[nodiscard]] std::vector<StateMachineNode>& getStateNodes() { return stateNodes; }
        [[nodiscard]] std::vector<StateMachineLink>& getStateLinks() { return stateLinks; }
        [[nodiscard]] NodeGraphRenderContext& getRenderContext() { return renderCtx; }

    protected:
        std::vector<NodeGraphNode> nodes;
        std::vector<CommentBox> comments;

		// State-machine component models.
		std::vector<StateMachineNode> stateNodes;
    std::vector<StateMachineLink> stateLinks;

        // Per-frame cached editor space mapping
        NodeGraphEditorSpace editorSpace;

        // Each NodeGraph needs its own ImNodes editor context.
        // If multiple graphs share the same ImNodes editor context, interactions such as
        // panning/zooming/selection will be shared and graphs will appear to move together.
        ImNodesEditorContext* editorCtx = nullptr;

        // Layered UI system
        NodeGraphViewRegistry views;
  NodeGraphRenderContext renderCtx{editorSpace,
    false, false, false, false, false, ImVec2(0.0f, 0.0f), NodeGraphInteractionState{},
    nodes, comments,
    stateNodes, stateLinks};

        ScreenSpaceUiFn screenSpaceUiFn = nullptr;
        void* screenSpaceUiUser = nullptr;

};


#endif //GLITTER_NODEGRAPH_HPP