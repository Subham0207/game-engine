//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODEGRAPH_NODESVIEW_HPP
#define GLITTER_NODEGRAPH_NODESVIEW_HPP
#pragma once
#include <vector>
#include <string>
#include <imgui.h>
#include <imnodes.h>
#include "../Components/NodeGraphNode.hpp"
#include "../Components/NodeGraphNodeLink.hpp"
#include "INodeGraphView.hpp"
#include "../NodeGraphRenderContext.hpp"
#include "NodeGraph/Components/NodeGraphIdAllocator.hpp"
#include "NodeGraph/Components/NodeGraphNodeFactory.hpp"
#include "NodeGraph/Components/NodeGraphNodes/NodeTypes.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Add.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/EqualsTo.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/GreaterThan.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/NotEqualsTo.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Subtract.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Boolean.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/GenericType.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Integer.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Function.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Print.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Return.hpp"
using AddNode = NodeGraphComponents::Node::Add;
using SubtractNode = NodeGraphComponents::Node::Subtract;
using GreaterThanNode = NodeGraphComponents::Node::GreaterThan;
using EqualsToNode = NodeGraphComponents::Node::EqualsTo;
using NotEqualsToNode = NodeGraphComponents::Node::NotEqualsTo;
using IntegerNode = NodeGraphComponents::Node::Integer;
using BooleanNode = NodeGraphComponents::Node::Boolean;
using GenericTypeNode = NodeGraphComponents::Node::GenericType;
using FunctionNode = NodeGraphComponents::Node::Keywords::Function;
using PrintNode = NodeGraphComponents::Node::Keywords::Print;
using ReturnNode = NodeGraphComponents::Node::Keywords::Return;
using NodeGraphComponents::NodeGraphIdAllocator;
using NodeGraphComponents::NodeGraphNodeFactory;

class NodeGraphNodesView final : public INodeGraphView
{
public:
    [[nodiscard]] NodeGraphLayer layer() const override { return NodeGraphLayer::Content; }

    NodeGraphNodesView(): idAllocator(), nodeFactory(&idAllocator)
    {
    }

    // Model mutation API (called by menu/tools). Keeping it here makes it easy to
    // add new element types with their own ID allocation logic.
    NodeGraphIdAllocator idAllocator;
    NodeGraphNodeFactory nodeFactory;

    NodeGraphNode* addFunctionNode(std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
                                   const std::vector<std::string>& argumentNames,
                                   const ImVec2& spawnPosScreen)
    {
        auto n = std::make_unique<FunctionNode>(
            idAllocator.nextOutputPinId,
            argumentNames,
            idAllocator.nextNodeId++,
            "Function",
            spawnPosScreen.x,
            spawnPosScreen.y
        );

        n->setSpawnPosScreen(spawnPosScreen);
        nodes.push_back(std::move(n));
        return nodes.back().get();
    }

    void addNode(std::vector<std::unique_ptr<NodeGraphNode>>& nodes, NodeTypes type, const ImVec2& spawnPosScreen)
    {
        nodeFactory.addNode(nodes, type, spawnPosScreen);
    }

    void addGenericTypeNode(std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
                            const std::string& objectName,
                            const std::vector<NodeGraphComponents::Node::GenericMemberSpec>& members,
                            const ImVec2& spawnPosScreen)
    {
        std::string title = "GenericType";
        if (!objectName.empty())
            title += "(" + objectName + ")";

        const bool asDestructuringNode = (objectName == "t");

        auto n = std::make_unique<GenericTypeNode>(
            idAllocator.nextInputPinId,
            idAllocator.nextOutputPinId,
            members,
            asDestructuringNode,
            idAllocator.nextNodeId++,
            title,
            spawnPosScreen.x,
            spawnPosScreen.y
        );

        n->setSpawnPosScreen(spawnPosScreen);
        nodes.push_back(std::move(n));
    }

	void addLink(std::vector<NodeGraphNodeLink>& links, int startAttr, int endAttr)
	{
    // Prevent duplicates (common when IsLinkCreated may fire on multiple frames depending on backend)
    for (const auto& l : links)
    {
      if (l.startAttr() == startAttr && l.endAttr() == endAttr)
        return;
    }
		links.emplace_back(idAllocator.nextLinkId++, startAttr, endAttr);
	}

    void draw(NodeGraphRenderContext& ctx) override
    {
        // If ImNodes is hovered/active, claim interaction ownership.
        // This keeps ImNodes-specific querying inside the nodes view.
        if (ctx.editorHovered)
        {
          const bool hoveringImNodesElement =
              (ctx.hoveredNodeId != -1) || (ctx.hoveredLinkId != -1) || (ctx.hoveredPinId != -1);
          if (hoveringImNodesElement || ctx.anyAttributeActive)
            ctx.interaction.tryClaim(NodeGraphInteractionOwner::ImNodes, 10);
        }

        auto& nodes = ctx.nodes;
    	auto& links = ctx.nodeGraphLinks;

        for (auto& node : nodes)
        {
            if (!node->positionSet())
            {
                const ImVec2 initial = node->hasSpawnPosScreen() ? node->spawnPosScreen() : ImGui::GetMousePos();
                ImNodes::SetNodeScreenSpacePos(node->id(), initial);
                node->markPositionSet(true);
            }

            ImNodes::BeginNode(node->id());

            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted(node->name().c_str());
            ImNodes::EndNodeTitleBar();

            if (node->hasExecInput())
            {
                auto execInput = node->getExecInput();
                const int attributeId = execInput->getId();
                ImNodes::BeginInputAttribute(attributeId, ImNodesPinShape_TriangleFilled);
                ImGui::Text(execInput->getName().c_str());
                //Input Attribute cannot have field.
                ImNodes::EndInputAttribute();
                ImGui::Spacing();
            }

            if (node->hasExecOutput())
            {
                auto execOutput = node->getExecOutput();
                const int attributeId = execOutput->getId();
                ImNodes::BeginOutputAttribute(attributeId, ImNodesPinShape_TriangleFilled);
                ImGui::Text(execOutput->getName().c_str());
                //Input Attribute cannot have field.
                ImNodes::EndOutputAttribute();
                ImGui::Spacing();
            }

            for (int i = 0; i < node->inputs().size(); ++i)
            {
                auto inputAttribute = node->inputs()[i];
                const int attributeId = inputAttribute.getId();

                ImNodes::BeginInputAttribute(attributeId);
                ImGui::Text(inputAttribute.getName().c_str());
                //Input Attribute cannot have field.
                ImNodes::EndInputAttribute();
                ImGui::Spacing();

            }

            for (int i = 0; i < node->outputs().size(); ++i)
            {
                auto& outputAttribute = node->outputs()[i];
                const int attributeId = node->outputs()[i].getId();

                ImNodes::BeginOutputAttribute(attributeId);
                ImGui::Text(outputAttribute.getName().c_str());
                if (outputAttribute.getType() == NodeAttributeType::FIELD)
                {
                    const auto fieldId = ("##NodeGraphNode" + std::to_string(i)).c_str();
                    ImGui::SetNextItemWidth(50.0f);
                    ImGui::InputText(fieldId ,outputAttribute.getValueBuff(), NodeGraphComponents::Node::Attribute::getValueSize());
                }
                ImNodes::EndOutputAttribute();
                ImGui::Spacing();
            }

            ImNodes::EndNode();
        }

    // Draw links after nodes so ImNodes can route them correctly.
    for (const auto& link : links)
    {
      ImNodes::Link(link.id(), link.startAttr(), link.endAttr());
    }

    }
};

#endif //GLITTER_NODEGRAPH_NODESVIEW_HPP
