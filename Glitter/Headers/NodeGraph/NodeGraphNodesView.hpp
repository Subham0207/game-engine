//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODEGRAPH_NODESVIEW_HPP
#define GLITTER_NODEGRAPH_NODESVIEW_HPP

#include <vector>
#include <string>
#include <imgui.h>
#include <imnodes.h>

#include "NodeGraphNode.hpp"
#include "INodeGraphView.hpp"
#include "NodeGraphRenderContext.hpp"

class NodeGraphNodesView final : public INodeGraphView
{
public:
    [[nodiscard]] NodeGraphLayer layer() const override { return NodeGraphLayer::Content; }

    // Model mutation API (called by menu/tools). Keeping it here makes it easy to
    // add new element types with their own ID allocation logic.
    int nextNodeId = 0;

    void addNode(std::vector<NodeGraphNode>& nodes, const std::string& name, const ImVec2& spawnPosScreen)
    {
        NodeGraphNode n(nextNodeId++, name, spawnPosScreen.x, spawnPosScreen.y);
        n.setSpawnPosScreen(spawnPosScreen);
        nodes.emplace_back(std::move(n));
    }

    void draw(NodeGraphRenderContext& ctx) override
    {
        // If ImNodes is hovered/active, claim interaction ownership.
        // This keeps ImNodes-specific querying inside the nodes view.
        if (ctx.editorHovered)
        {
          int hoveredNode = -1;
          int hoveredLink = -1;
          int hoveredPin = -1;
          (void)ImNodes::IsNodeHovered(&hoveredNode);
          (void)ImNodes::IsLinkHovered(&hoveredLink);
          (void)ImNodes::IsPinHovered(&hoveredPin);

          int activeAttribute = -1;
          const bool anyAttributeActive = ImNodes::IsAnyAttributeActive(&activeAttribute);
          const bool hoveringImNodesElement = (hoveredNode != -1) || (hoveredLink != -1) || (hoveredPin != -1);

          if (hoveringImNodesElement || anyAttributeActive)
            ctx.interaction.tryClaim(NodeGraphInteractionOwner::ImNodes, 10);
        }

        auto& nodes = ctx.nodes;

        for (auto& node : nodes)
        {
            if (!node.positionSet())
            {
                const ImVec2 initial = node.hasSpawnPosScreen() ? node.spawnPosScreen() : ImGui::GetMousePos();
                ImNodes::SetNodeScreenSpacePos(node.id(), initial);
                node.markPositionSet(true);
            }

            ImNodes::BeginNode(node.id());

            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted(node.name().c_str());
            ImNodes::EndNodeTitleBar();

            ImNodes::BeginInputAttribute(node.id() * 1000 + 1);
            ImGui::Text("Input");
            ImNodes::EndInputAttribute();

            ImGui::Spacing();

            ImNodes::BeginOutputAttribute(node.id() * 1000 + 2);
            ImGui::Indent(40.f);
            ImGui::Text("Output");
            ImNodes::EndOutputAttribute();

            ImNodes::EndNode();
        }
    }
};

#endif //GLITTER_NODEGRAPH_NODESVIEW_HPP



