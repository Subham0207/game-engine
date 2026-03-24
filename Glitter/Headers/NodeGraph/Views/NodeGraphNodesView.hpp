//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODEGRAPH_NODESVIEW_HPP
#define GLITTER_NODEGRAPH_NODESVIEW_HPP

#include <vector>
#include <string>
#include <imgui.h>
#include <imnodes.h>
#include "../Components/NodeGraphNode.hpp"
#include "../Components/NodeGraphNodeLink.hpp"
#include "../NodeGraphIdRanges.hpp"
#include "INodeGraphView.hpp"
#include "../NodeGraphRenderContext.hpp"

class NodeGraphNodesView final : public INodeGraphView
{
public:
    [[nodiscard]] NodeGraphLayer layer() const override { return NodeGraphLayer::Content; }

    // Model mutation API (called by menu/tools). Keeping it here makes it easy to
    // add new element types with their own ID allocation logic.
    int nextNodeId = NodeGraphIdBase(NodeGraphElementIdBase::NodeGraphNode);
	int nextLinkId = NodeGraphIdBase(NodeGraphElementIdBase::NodeGraphNodeLink);

    void addNode(std::vector<NodeGraphNode>& nodes, const std::string& name, const ImVec2& spawnPosScreen)
    {
        NodeGraphNode n(nextNodeId++, name, spawnPosScreen.x, spawnPosScreen.y);
        n.setSpawnPosScreen(spawnPosScreen);
        nodes.emplace_back(std::move(n));
    }

	void addLink(std::vector<NodeGraphNodeLink>& links, int startAttr, int endAttr)
	{
    // Prevent duplicates (common when IsLinkCreated may fire on multiple frames depending on backend)
    for (const auto& l : links)
    {
      if (l.startAttr() == startAttr && l.endAttr() == endAttr)
        return;
    }
		links.emplace_back(nextLinkId++, startAttr, endAttr);
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
    	auto& links = ctx.nodeGraphLinks;

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

            ImNodes::BeginInputAttribute(node.id() * static_cast<int>(NodeGraphElementIdBase::NodeGraphNodeLink) + 1);
            ImGui::Text("Input");
            ImNodes::EndInputAttribute();

            ImGui::Spacing();

            ImNodes::BeginOutputAttribute(node.id() * static_cast<int>(NodeGraphElementIdBase::NodeGraphNodeLink) + 2);
            ImGui::Indent(40.f);
            ImGui::Text("Output");
            ImNodes::EndOutputAttribute();

            ImNodes::EndNode();
        }

    // Draw links after nodes so ImNodes can route them correctly.
    for (const auto& link : links)
    {
      ImNodes::Link(link.id(), link.startAttr(), link.endAttr());
    }

    // Allow users to create new links by dragging between attributes.
    // NOTE: This project uses a vendored imnodes version; in this build the
    // available overload is IsLinkCreated(int* start_attr, int* end_attr).
    // int startAttr = -1;
    // int endAttr = -1;
    // if (ImNodes::IsLinkCreated(&startAttr, &endAttr))
    // {
    //   addLink(links, startAttr, endAttr);
    // }
    }
};

#endif //GLITTER_NODEGRAPH_NODESVIEW_HPP



