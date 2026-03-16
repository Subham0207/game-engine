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

class NodeGraphNodesView
{
public:
    explicit NodeGraphNodesView(std::vector<NodeGraphNode>& nodes)
        : m_nodes(nodes)
    {
    }

    void draw()
    {
        for (auto& node : m_nodes)
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

private:
    std::vector<NodeGraphNode>& m_nodes;
};

#endif //GLITTER_NODEGRAPH_NODESVIEW_HPP


