//
// Created by subha on 28-04-2026.
//

#include <gtest/gtest.h>
#include <NodeGraph/FlowScript/Compile/Ast.hpp>

#include "NodeGraph/Components/NodeGraphNodeFactory.hpp"
#include "NodeGraph/Components/NodeGraphNodeLink.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Add.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/EqualsTo.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Multiply.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/LessThan.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Integer.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Function.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Print.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Return.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Variables/GetVariable.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Variables/VariableDeclaration.hpp"
#include "NodeGraph/FlowScript/Compile/Compiler.hpp"

using NodeGraphComponents::Node::Integer;
using NodeGraphComponents::Node::Add;
using NodeGraphComponents::Node::EqualsTo;
using NodeGraphComponents::Node::Keywords::Function;
using NodeGraphComponents::Node::Keywords::Print;
using NodeGraphComponents::Node::Keywords::Return;
using NodeGraphComponents::NodeGraphNodeFactory;
using NodeGraphComponents::NodeGraphIdAllocator;
using Flowscript::Compile::FunctionStatementAstNode;
using Flowscript::Compile::PrintStatementAstNode;
using Flowscript::Compile::ReturnStatementAstNode;
using Flowscript::Compile::EqualsToExpressionAstNode;
using Flowscript::Compile::IntegerLiteralExpressionAstNode;
using Flowscript::Compile::VariableDeclarationStatementAstNode;
using Flowscript::Compile::GetVariableExpressionAstNode;
using Flowscript::Compile::MultiplyExpressionAstNode;
using Flowscript::Compile::LessThanExpressionAstNode;

TEST(Ast, shouldCreateTree)
{
    // function node -> print node -> return node.
    int nextLinkId = 1;

    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    std::vector<NodeGraphNodeLink> links;

    auto nodeGraphIdAllocator = new NodeGraphIdAllocator();
    auto nodeGraphFactory = new NodeGraphNodeFactory(nodeGraphIdAllocator);

    auto fn = nodeGraphFactory->addFunctionNode(nodes, std::vector<std::string>{"x"});
    auto* fnNode = dynamic_cast<NodeGraphComponents::Node::Keywords::Function*>(fn);
    ASSERT_NE(fnNode, nullptr);
    fnNode->functionName = "foo";
    auto print = nodeGraphFactory->addNode(nodes, NodeTypes::Print);
    auto ret = nodeGraphFactory->addNode(nodes, NodeTypes::Return);

    //TODO: add simple functions for test to attach exec flow nodes.
    links.emplace_back(nextLinkId++, fn->getExecOutputId(), print->getExecInputId());
    links.emplace_back(nextLinkId++, print->getExecOutputId(), ret->getExecInputId());

    const Flowscript::Compile::Ast ast(nodes, links);
    ASSERT_EQ(ast.programRoot.size(), 1u);

    const auto* rootNode = ast.programRoot[0].get();
    ASSERT_NE(rootNode, nullptr);
    const auto* functionAstNode = dynamic_cast<const FunctionStatementAstNode*>(rootNode);
    ASSERT_NE(functionAstNode, nullptr);
    EXPECT_EQ(functionAstNode->functionName, "foo");
    ASSERT_EQ(functionAstNode->parameters.size(), 1u);
    EXPECT_EQ(functionAstNode->parameters[0], "x");

    ASSERT_EQ(rootNode->outputExecutionFlows.size(), 1u);
    const auto* printAstNode = rootNode->outputExecutionFlows[0].get();
    ASSERT_NE(printAstNode, nullptr);
    EXPECT_NE(dynamic_cast<const PrintStatementAstNode*>(printAstNode), nullptr);

    ASSERT_EQ(printAstNode->outputExecutionFlows.size(), 1u);
    const auto* returnAstNode = printAstNode->outputExecutionFlows[0].get();
    ASSERT_NE(returnAstNode, nullptr);
    EXPECT_NE(dynamic_cast<const ReturnStatementAstNode*>(returnAstNode), nullptr);
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
    const auto* printAstNode = dynamic_cast<const PrintStatementAstNode*>(rootNode);
    ASSERT_NE(printAstNode, nullptr);

    ASSERT_NE(printAstNode->expression, nullptr);
    const auto* equalsToNode = dynamic_cast<const EqualsToExpressionAstNode*>(printAstNode->expression.get());
    ASSERT_NE(equalsToNode, nullptr);
    const auto* integer1_ast = dynamic_cast<const IntegerLiteralExpressionAstNode*>(equalsToNode->lhs.get());
    const auto* integer2_ast = dynamic_cast<const IntegerLiteralExpressionAstNode*>(equalsToNode->rhs.get());

    ASSERT_NE(integer1_ast, nullptr);
    EXPECT_EQ(integer1_ast->value, "1");

    ASSERT_NE(integer2_ast, nullptr);
    EXPECT_EQ(integer2_ast->value, "2");
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

TEST(Ast, shouldCreateVariableDeclarationAndGetVariableTree)
{
    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    std::vector<NodeGraphNodeLink> links;
    int nextLinkId = 1;

    auto nodeGraphIdAllocator = new NodeGraphIdAllocator();
    auto nodeGraphFactory = new NodeGraphNodeFactory(nodeGraphIdAllocator);

    auto fn = nodeGraphFactory->addFunctionNode(nodes, std::vector<std::string>{});
    auto* fnNode = dynamic_cast<NodeGraphComponents::Node::Keywords::Function*>(fn);
    ASSERT_NE(fnNode, nullptr);
    fnNode->functionName = "vars";

    auto varDecl = nodeGraphFactory->addNode(nodes, NodeTypes::VariableDeclaration);
    auto getVar = nodeGraphFactory->addNode(nodes, NodeTypes::GetVariable);
    auto printNode = nodeGraphFactory->addNode(nodes, NodeTypes::Print);

    auto* varDeclNode = dynamic_cast<NodeGraphComponents::Node::Variables::VariableDeclaration*>(varDecl);
    ASSERT_NE(varDeclNode, nullptr);
    varDeclNode->variableName = "score";
    varDeclNode->declaredType = "Number";
    varDeclNode->value = "42";

    auto* getVarNode = dynamic_cast<NodeGraphComponents::Node::Variables::GetVariable*>(getVar);
    ASSERT_NE(getVarNode, nullptr);
    getVarNode->variableName = "score";

    links.emplace_back(nextLinkId++, fn->getExecOutputId(), varDecl->getExecInputId());
    links.emplace_back(nextLinkId++, varDecl->getExecOutputId(), printNode->getExecInputId());
    links.emplace_back(nextLinkId++, getVar->outputs()[0].getId(), printNode->inputs()[0].getId());

    const Flowscript::Compile::Ast ast(nodes, links);
    ASSERT_EQ(ast.programRoot.size(), 1u);

    const auto* rootNode = ast.programRoot[0].get();
    const auto* functionAstNode = dynamic_cast<const FunctionStatementAstNode*>(rootNode);
    ASSERT_NE(functionAstNode, nullptr);
    ASSERT_EQ(functionAstNode->outputExecutionFlows.size(), 1u);

    const auto* declAstNode = dynamic_cast<const VariableDeclarationStatementAstNode*>(functionAstNode->outputExecutionFlows[0].get());
    ASSERT_NE(declAstNode, nullptr);
    EXPECT_EQ(declAstNode->name, "score");
    EXPECT_EQ(declAstNode->valueType, Flowscript::Compile::VariableValueType::Number);
    EXPECT_EQ(declAstNode->value, "42");

    ASSERT_EQ(declAstNode->outputExecutionFlows.size(), 1u);
    const auto* printAstNode = dynamic_cast<const PrintStatementAstNode*>(declAstNode->outputExecutionFlows[0].get());
    ASSERT_NE(printAstNode, nullptr);
    ASSERT_NE(printAstNode->expression, nullptr);
    const auto* getVarAstNode = dynamic_cast<const GetVariableExpressionAstNode*>(printAstNode->expression.get());
    ASSERT_NE(getVarAstNode, nullptr);
    EXPECT_EQ(getVarAstNode->name, "score");
}

TEST(Ast, shouldCaptureNodePositionsAndNewBinaryOperators)
{
    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    std::vector<NodeGraphNodeLink> links;
    int nextLinkId = 1;

    auto nodeGraphIdAllocator = new NodeGraphIdAllocator();
    auto nodeGraphFactory = new NodeGraphNodeFactory(nodeGraphIdAllocator);

    auto fn = nodeGraphFactory->addFunctionNode(nodes, std::vector<std::string>{"x"}, ImVec2(100.0f, 200.0f));
    auto* fnNode = dynamic_cast<NodeGraphComponents::Node::Keywords::Function*>(fn);
    ASSERT_NE(fnNode, nullptr);
    fnNode->functionName = "positioned";

    auto printNode = nodeGraphFactory->addNode(nodes, NodeTypes::Print, ImVec2(120.0f, 220.0f));
    auto lessThanNode = nodeGraphFactory->addNode(nodes, NodeTypes::LessThan, ImVec2(140.0f, 240.0f));
    auto multiplyNode = nodeGraphFactory->addNode(nodes, NodeTypes::Multiply, ImVec2(160.0f, 260.0f));

    auto integer1 = nodeGraphFactory->addNode(nodes, NodeTypes::Integer, ImVec2(180.0f, 280.0f));
    integer1->outputs()[0].setValue("2");
    auto integer2 = nodeGraphFactory->addNode(nodes, NodeTypes::Integer, ImVec2(200.0f, 300.0f));
    integer2->outputs()[0].setValue("3");
    auto integer3 = nodeGraphFactory->addNode(nodes, NodeTypes::Integer, ImVec2(220.0f, 320.0f));
    integer3->outputs()[0].setValue("7");

    links.emplace_back(nextLinkId++, fn->getExecOutputId(), printNode->getExecInputId());
    links.emplace_back(nextLinkId++, multiplyNode->outputs()[0].getId(), lessThanNode->inputs()[0].getId());
    links.emplace_back(nextLinkId++, integer3->outputs()[0].getId(), lessThanNode->inputs()[1].getId());
    links.emplace_back(nextLinkId++, integer1->outputs()[0].getId(), multiplyNode->inputs()[0].getId());
    links.emplace_back(nextLinkId++, integer2->outputs()[0].getId(), multiplyNode->inputs()[1].getId());
    links.emplace_back(nextLinkId++, lessThanNode->outputs()[0].getId(), printNode->inputs()[0].getId());

    const Flowscript::Compile::Ast ast(nodes, links);
    ASSERT_EQ(ast.programRoot.size(), 1u);

    const auto* functionAst = dynamic_cast<const FunctionStatementAstNode*>(ast.programRoot[0].get());
    ASSERT_NE(functionAst, nullptr);
    EXPECT_FLOAT_EQ(functionAst->x, 100.0f);
    EXPECT_FLOAT_EQ(functionAst->y, 200.0f);

    ASSERT_EQ(functionAst->outputExecutionFlows.size(), 1u);
    const auto* printAst = dynamic_cast<const PrintStatementAstNode*>(functionAst->outputExecutionFlows[0].get());
    ASSERT_NE(printAst, nullptr);
    EXPECT_FLOAT_EQ(printAst->x, 120.0f);
    EXPECT_FLOAT_EQ(printAst->y, 220.0f);

    const auto* lessThanAst = dynamic_cast<const LessThanExpressionAstNode*>(printAst->expression.get());
    ASSERT_NE(lessThanAst, nullptr);
    EXPECT_FLOAT_EQ(lessThanAst->x, 140.0f);
    EXPECT_FLOAT_EQ(lessThanAst->y, 240.0f);

    const auto* multiplyAst = dynamic_cast<const MultiplyExpressionAstNode*>(lessThanAst->lhs.get());
    ASSERT_NE(multiplyAst, nullptr);
    EXPECT_FLOAT_EQ(multiplyAst->x, 160.0f);
    EXPECT_FLOAT_EQ(multiplyAst->y, 260.0f);
}
