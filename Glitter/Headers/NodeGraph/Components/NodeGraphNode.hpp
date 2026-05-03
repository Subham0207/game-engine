//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODE_HPP
#define GLITTER_NODE_HPP

#pragma once

#include <string>
#include <utility>

#include <imgui.h>
#include "Attribute.hpp"
#include "NodeGraphNodes/NodeTypes.hpp"

using NodeAttribute = NodeGraphComponents::Node::Attribute;
using NodeAttributeType = NodeGraphComponents::Node::TYPE;

// Node type used by the NodeGraph editor.
// NOTE: named NodeGraphNode to avoid collisions with any other "Node" types.
class NodeGraphNode
{
public:
    virtual ~NodeGraphNode() = default;
    NodeGraphNode() = default;
    NodeGraphNode(int id, std::string name, float x = 0.0f, float y = 0.0f)
        : m_id(id), m_name(std::move(name)), m_spawnPosScreen(x,y), executionFlowIn(nullptr), executionFlowOut(nullptr)
    {
    }

    [[nodiscard]] int id() const { return m_id; }
    [[nodiscard]] const std::string& name() const { return m_name; }

    [[nodiscard]] bool positionSet() const { return m_positionSet; }
    void markPositionSet(bool set = true) { m_positionSet = set; }

    // When a node is created via context menu, we store its desired spawn position
    // in screen space and apply it once on first draw.
    void setSpawnPosScreen(const ImVec2& p)
    {
        m_spawnPosScreen = p;
        m_hasSpawnPos = true;
    }
    [[nodiscard]] bool hasSpawnPosScreen() const { return m_hasSpawnPos; }
    [[nodiscard]] ImVec2 spawnPosScreen() const { return m_spawnPosScreen; }

    std::vector<NodeAttribute>& inputs() { return inputAttributes; }
    std::vector<NodeAttribute>& outputs() { return outputAttributes; }

    void setupExecInput(int& nextInputPinId, const std::string& name = "")
    {
        executionFlowIn = std::make_unique<NodeAttribute>(nextInputPinId++, name, NodeAttributeType::ExecutionFlowInPin);
    }

    void setupExecOutput(int& nextOutputPinId, const std::string& name = "")
    {
        executionFlowOut = std::make_unique<NodeAttribute>(nextOutputPinId++, name, NodeAttributeType::ExecutionFlowOutPin);
    }

    bool hasExecInput() const { return executionFlowIn != nullptr; }
    bool hasExecOutput() const { return executionFlowOut != nullptr; }

    NodeAttribute* getExecInput() const { return executionFlowIn.get(); }
    NodeAttribute* getExecOutput() const { return executionFlowOut.get(); }

    [[nodiscard]] int getExecInputId() const { return executionFlowIn ? executionFlowIn->getId() : -1; }
    [[nodiscard]] int getExecOutputId() const { return executionFlowOut ? executionFlowOut->getId(): -1; }

    virtual NodeTypes type() = 0;

private:
    int m_id = -1;
    std::string m_name;

    bool m_positionSet = false;

    bool m_hasSpawnPos = false;
    ImVec2 m_spawnPosScreen{0.0f, 0.0f};

    std::vector<NodeAttribute> inputAttributes;
    std::vector<NodeAttribute> outputAttributes;

    //For making this node part of an execution flow
    //connect any outgoing execution to executionFlowIn.
    //And to continue the execution flow after this node
    //connect the executionFlowOut to another executionFlowIn.
    std::unique_ptr<NodeAttribute> executionFlowIn;
    std::unique_ptr<NodeAttribute> executionFlowOut;
};


#endif //GLITTER_NODE_HPP