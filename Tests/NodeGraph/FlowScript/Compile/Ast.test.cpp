//
// Created by subha on 28-04-2026.
//

#include <gtest/gtest.h>
#include <NodeGraph/FlowScript/Compile/Ast.hpp>

#include "NodeGraph/Components/NodeGraphNodeLink.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Add.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/EqualsTo.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Integer.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Function.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Print.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Return.hpp"
#include "NodeGraph/FlowScript/Compile/Ast.hpp"
#include "NodeGraph/FlowScript/Compile/Compiler.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Add.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/EqualsTo.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Integer.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Function.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Print.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Return.hpp"

using NodeGraphComponents::Node::Integer;
using NodeGraphComponents::Node::Add;
using NodeGraphComponents::Node::EqualsTo;
using NodeGraphComponents::Node::Keywords::Function;
using NodeGraphComponents::Node::Keywords::Print;
using NodeGraphComponents::Node::Keywords::Return;

TEST(Ast, shouldCreateTree)
{
    // function node -> print node -> return node.
    int nextInputPinId = 1000;
    int nextOutputPinId = 1;
    int nextLinkId = 1;

    auto functionNode = std::make_unique<Function>(nextOutputPinId, std::vector<std::string>{"x"}, 10, "Function");
    auto printNode = std::make_unique<Print>(nextInputPinId, nextOutputPinId, 20, "Print");
    auto returnNode = std::make_unique<Return>(nextInputPinId, 30, "Return");

    NodeGraphNode* fn = functionNode.get();
    NodeGraphNode* print = printNode.get();
    NodeGraphNode* ret = returnNode.get();

    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    nodes.push_back(std::move(functionNode));
    nodes.push_back(std::move(printNode));
    nodes.push_back(std::move(returnNode));

    std::vector<NodeGraphNodeLink> links;
    links.emplace_back(nextLinkId++, fn->getExecOutput()->getId(), print->getExecInput()->getId());
    links.emplace_back(nextLinkId++, print->getExecOutput()->getId(), ret->getExecInput()->getId());

    const Flowscript::Compile::Ast ast(nodes, links);
    ASSERT_EQ(ast.programRoot.size(), 1u);

    const auto* rootNode = ast.programRoot[0].get();
    ASSERT_NE(rootNode, nullptr);
    EXPECT_EQ(rootNode->type, "Function");

    ASSERT_EQ(rootNode->children.size(), 1u);
    const auto* printAstNode = rootNode->children[0].get();
    ASSERT_NE(printAstNode, nullptr);
    EXPECT_EQ(printAstNode->type, "Print");

    ASSERT_EQ(printAstNode->children.size(), 1u);
    const auto* returnAstNode = printAstNode->children[0].get();
    ASSERT_NE(returnAstNode, nullptr);
    EXPECT_EQ(returnAstNode->type, "Return");
}

// TEST(Ast, shouldCreateTreeFromNodesAndLinks)
// {
//     std::vector<std::unique_ptr<NodeGraphNode>> nodes;
//
//     //c = a + b;
//     int nextInputPinId = 1000;
//     int nextOutputPinId = 1;
//     int nextLinkId = 1;
//
//     // literals
//     auto integer1 = std::make_unique<Integer>(nextOutputPinId, 101, "Integer1");
//     integer1->outputs()[0].setValue("1");
//
//     auto integer2 = std::make_unique<Integer>(nextOutputPinId, 102, "Integer2");
//     integer2->outputs()[0].setValue("2");
//
//     // comparison node (needs BOTH counters)
//     auto equals = std::make_unique<EqualsTo>(nextInputPinId, nextOutputPinId, 103, "EqualsTo");
//
//     // wire: int1 -> FirstValue, int2 -> SecondValue
//     std::vector<NodeGraphNodeLink> links;
//     links.emplace_back(nextLinkId++, integer1->outputs()[0].getId(), equals->inputs()[0].getId());
//     links.emplace_back(nextLinkId++, integer2->outputs()[0].getId(), equals->inputs()[1].getId());
// }