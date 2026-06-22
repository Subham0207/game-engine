//
// Created by subha on 01-05-2026.
//

#include "../Headers/NodeGraph/Components/NodeGraphNodeFactory.hpp"

#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Add.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/EqualsTo.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/GreaterThan.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/NotEqualsTo.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Subtract.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/LessThan.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Modulo.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Multiply.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Divide.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Boolean.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/GenericType.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Integer.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Function.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Print.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Return.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Variables/GetVariable.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Variables/VariableDeclaration.hpp"
using AddNode = NodeGraphComponents::Node::Add;
using SubtractNode = NodeGraphComponents::Node::Subtract;
using GreaterThanNode = NodeGraphComponents::Node::GreaterThan;
using EqualsToNode = NodeGraphComponents::Node::EqualsTo;
using NotEqualsToNode = NodeGraphComponents::Node::NotEqualsTo;
using IntegerNode = NodeGraphComponents::Node::Integer;
using BooleanNode = NodeGraphComponents::Node::Boolean;
using FunctionNode = NodeGraphComponents::Node::Keywords::Function;
using PrintNode = NodeGraphComponents::Node::Keywords::Print;
using ReturnNode = NodeGraphComponents::Node::Keywords::Return;
using GetVariableNode = NodeGraphComponents::Node::Variables::GetVariable;
using VariableDeclarationNode = NodeGraphComponents::Node::Variables::VariableDeclaration;
using MultiplyNode = NodeGraphComponents::Node::Multiply;
using DivideNode = NodeGraphComponents::Node::Divide;
using ModuloNode = NodeGraphComponents::Node::Modulo;
using LessThanNode = NodeGraphComponents::Node::LessThan;

namespace NodeGraphComponents
{
    NodeGraphNode* NodeGraphNodeFactory::addNode
    (
        std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
        NodeTypes type,
        const ImVec2& spawnPosScreen
    )
    {
        std::unique_ptr<NodeGraphNode> n;
        if (type == NodeTypes::Add)
        {
            n = std::make_unique<AddNode>(_nodeGraphIdAllocator->nextInputPinId, _nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "Add", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::Subtract)
        {
            n = std::make_unique<SubtractNode>(_nodeGraphIdAllocator->nextInputPinId, _nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "Subtract", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::Multiply)
        {
            n = std::make_unique<MultiplyNode>(_nodeGraphIdAllocator->nextInputPinId, _nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "Multiply", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::Divide)
        {
            n = std::make_unique<DivideNode>(_nodeGraphIdAllocator->nextInputPinId, _nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "Divide", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::Modulo)
        {
            n = std::make_unique<ModuloNode>(_nodeGraphIdAllocator->nextInputPinId, _nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "Modulo", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::LessThan)
        {
            n = std::make_unique<LessThanNode>(_nodeGraphIdAllocator->nextInputPinId, _nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "LessThan", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::GreaterThan)
        {
            n = std::make_unique<GreaterThanNode>(_nodeGraphIdAllocator->nextInputPinId, _nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "GreaterThan", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::EqualsTo)
        {
            n = std::make_unique<EqualsToNode>(_nodeGraphIdAllocator->nextInputPinId, _nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "EqualsTo", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::NotEqualsTo)
        {
            n = std::make_unique<NotEqualsToNode>(_nodeGraphIdAllocator->nextInputPinId, _nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "NotEqualsTo", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::Integer)
        {
            n = std::make_unique<IntegerNode>(_nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "Integer", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::Boolean)
        {
            n = std::make_unique<BooleanNode>(_nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "Boolean", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::Function)
        {
            n = std::make_unique<FunctionNode>(_nodeGraphIdAllocator->nextOutputPinId, std::vector<std::string>{"t"}, _nodeGraphIdAllocator->nextNodeId++, "Function", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::Print)
        {
            n = std::make_unique<PrintNode>(_nodeGraphIdAllocator->nextInputPinId, _nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "Print", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::Return)
        {
            n = std::make_unique<ReturnNode>(_nodeGraphIdAllocator->nextInputPinId, _nodeGraphIdAllocator->nextNodeId++, "Return", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::VariableDeclaration)
        {
            n = std::make_unique<VariableDeclarationNode>(_nodeGraphIdAllocator->nextInputPinId, _nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "VariableDeclaration", spawnPosScreen.x, spawnPosScreen.y);
        }
        if (type == NodeTypes::GetVariable)
        {
            n = std::make_unique<GetVariableNode>(_nodeGraphIdAllocator->nextOutputPinId, _nodeGraphIdAllocator->nextNodeId++, "GetVariable", spawnPosScreen.x, spawnPosScreen.y);
        }


        if (n)
        {
            n->setSpawnPosScreen(spawnPosScreen);
            nodes.push_back(std::move(n));
        }
        return nodes.back().get();
    }

    NodeGraphNode* NodeGraphNodeFactory::addFunctionNode(
        std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
        const std::vector<std::string>& args,
        ImVec2 spawnPosScreen
    )
    {
        std::unique_ptr<NodeGraphNode> n = std::make_unique<FunctionNode>(
            _nodeGraphIdAllocator->nextOutputPinId,
            args,
            _nodeGraphIdAllocator->nextNodeId++,
            "Function",
            spawnPosScreen.x,
            spawnPosScreen.y);
        n->setSpawnPosScreen(spawnPosScreen);
        nodes.push_back(std::move(n));
        return nodes.back().get();
    }

} // NodeGraphComponents

