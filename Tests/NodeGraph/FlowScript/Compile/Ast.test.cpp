//
// Created by subha on 28-04-2026.
//

#include <gtest/gtest.h>
#include <NodeGraph/FlowScript/Compile/Ast.hpp>

#include "NodeGraph/Components/NodeGraphNodeFactory.hpp"
#include "NodeGraph/Components/NodeGraphNodeLink.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Add.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/EqualsTo.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Integer.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Function.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Print.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Return.hpp"
#include "NodeGraph/FlowScript/Compile/Compiler.hpp"

using NodeGraphComponents::Node::Integer;
using NodeGraphComponents::Node::Add;
using NodeGraphComponents::Node::EqualsTo;
using NodeGraphComponents::Node::Keywords::Function;
using NodeGraphComponents::Node::Keywords::Print;
using NodeGraphComponents::Node::Keywords::Return;
using NodeGraphComponents::NodeGraphNodeFactory;
using NodeGraphComponents::NodeGraphIdAllocator;

TEST(Ast, shouldCreateTree)
{
    // function node -> print node -> return node.
    int nextLinkId = 1;

    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    std::vector<NodeGraphNodeLink> links;

    auto nodeGraphIdAllocator = new NodeGraphIdAllocator();
    auto nodeGraphFactory = new NodeGraphNodeFactory(nodeGraphIdAllocator);

    auto fn = nodeGraphFactory->addFunctionNode(nodes, std::vector<std::string>{"x"});
    auto print = nodeGraphFactory->addNode(nodes, NodeTypes::Print);
    auto ret = nodeGraphFactory->addNode(nodes, NodeTypes::Return);

    //TODO: add simple functions for test to attach exec flow nodes.
    links.emplace_back(nextLinkId++, fn->getExecOutputId(), print->getExecInputId());
    links.emplace_back(nextLinkId++, print->getExecOutputId(), ret->getExecInputId());

    const Flowscript::Compile::Ast ast(nodes, links);
    ASSERT_EQ(ast.programRoot.size(), 1u);

    const auto* rootNode = ast.programRoot[0].get();
    ASSERT_NE(rootNode, nullptr);
    EXPECT_EQ(rootNode->type, "Function");

    ASSERT_EQ(rootNode->outputExecutionFlows.size(), 1u);
    const auto* printAstNode = rootNode->outputExecutionFlows[0].get();
    ASSERT_NE(printAstNode, nullptr);
    EXPECT_EQ(printAstNode->type, "Print");

    ASSERT_EQ(printAstNode->outputExecutionFlows.size(), 1u);
    const auto* returnAstNode = printAstNode->outputExecutionFlows[0].get();
    ASSERT_NE(returnAstNode, nullptr);
    EXPECT_EQ(returnAstNode->type, "Return");
}

TEST(Ast, shouldCreateTreeFromNodesAndLinks)
{
    std::vector<std::unique_ptr<NodeGraphNode>> nodes;

    //c = a + b;
    int nextLinkId = 1;

    auto nodeGraphIdAllocator = new NodeGraphComponents::NodeGraphIdAllocator();
    auto nodeGraphFactory = new NodeGraphNodeFactory(nodeGraphIdAllocator);

    // literals
    auto integer1 = nodeGraphFactory->addNode(nodes, NodeTypes::Integer);
    integer1->outputs()[0].setValue("1");

    auto integer2 = nodeGraphFactory->addNode(nodes, NodeTypes::Integer);
    integer2->outputs()[0].setValue("2");

    // comparison node (needs BOTH counters)
    auto equals = nodeGraphFactory->addNode(nodes, NodeTypes::EqualsTo);

    auto print = nodeGraphFactory->addNode(nodes, NodeTypes::Print);

    // wire: int1 -> FirstValue, int2 -> SecondValue
    std::vector<NodeGraphNodeLink> links;
    links.emplace_back(nextLinkId++, integer1->outputs()[0].getId(), equals->inputs()[0].getId());
    links.emplace_back(nextLinkId++, integer2->outputs()[0].getId(), equals->inputs()[1].getId());
    links.emplace_back(nextLinkId++, equals->outputs()[0].getId(), print->inputs()[0].getId());

    const Flowscript::Compile::Ast ast(nodes, links);
    ASSERT_EQ(ast.programRoot.size(), 1u); // tree resolution worked here. need to add equalsTo, integer Node in ast.

    const auto* rootNode = ast.programRoot[0].get();
    ASSERT_NE(rootNode, nullptr);
    EXPECT_EQ(rootNode->type, "Print");

    ASSERT_EQ(rootNode->inputDataChildrens.size(), 1u);
    const auto* equalsToNode = rootNode->inputDataChildrens[0].get();
    ASSERT_NE(equalsToNode, nullptr);
    EXPECT_EQ(equalsToNode->type, "EqualsTo");

    ASSERT_EQ(equalsToNode->inputDataChildrens.size(), 2u);
    const auto* integer1_ast = equalsToNode->inputDataChildrens[0].get();
    const auto* integer2_ast = equalsToNode->inputDataChildrens[1].get();

    //TODO: update type checking here...
    ASSERT_NE(integer1_ast, nullptr);
    EXPECT_EQ(integer1_ast->type, "Integer");

    ASSERT_NE(integer2_ast, nullptr);
    EXPECT_EQ(integer2_ast->type, "Integer");
}

TEST(Ast, EasyTreeTest)
{
    //NodeGraphIdAllocator: owns nextNodeId, nextInputPinId, nextOutputPinId, nextLinkId.
    //pass instance of this when creating NodeGraphNodeFactory. Now we don't need to manually manage this.

    //NodeGraphNodeFactory creates node of a certain type: (nodes, nodeType, optional screenPos);
    //(NOT IMPORTANT: ) nodes contains all the nodes. if we can make the factory own nodes. this becomes easier.

    //NodeGraphNodeLinkFactory create link b/w two nodes.

    //why does the nodes need name again when they have an id ? For identifier type nodes we can pass name ( example for variable, function names, etc)
}