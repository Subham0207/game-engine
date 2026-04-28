#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "NodeGraph/Components/NodeGraphNode.hpp"
#include "NodeGraph/Components/NodeGraphNodeLink.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Boolean.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Integer.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Function.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Print.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Return.hpp"
#include "NodeGraph/FlowScript/Compile/Compiler.hpp"
#include "NodeGraph/FlowScript/Compile/IntermediateRepresentation/Expression.hpp"
#include "NodeGraph/FlowScript/Compile/IntermediateRepresentation/Statement.hpp"

using Flowscript::Compile::BoolExpr;
using Flowscript::Compile::Compiler;
using Flowscript::Compile::FunctionStmt;
using Flowscript::Compile::LocalAssignStmt;
using Flowscript::Compile::NumberExpr;
using Flowscript::Compile::PrintStmt;
using Flowscript::Compile::ReturnStmt;
using Flowscript::Compile::VariableExpr;
using NodeGraphComponents::Node::Boolean;
using NodeGraphComponents::Node::Integer;
using NodeGraphComponents::Node::Keywords::Function;
using NodeGraphComponents::Node::Keywords::Print;
using NodeGraphComponents::Node::Keywords::Return;

TEST(CompilerEmitPureDataExpressionAssignments, shouldEmitDataExpressionAssignments)
{
    int nextOutputPinId = 1;

    std::vector<std::unique_ptr<NodeGraphNode>> nodes;

    auto integerNode = std::make_unique<Integer>(nextOutputPinId, 101, "Integer");
    integerNode->outputs()[0].setValue("42");

    auto booleanNode = std::make_unique<Boolean>(nextOutputPinId, 202, "Boolean");
    booleanNode->outputs()[0].setValue("true");

    nodes.push_back(std::move(integerNode));
    nodes.push_back(std::move(booleanNode));

    std::vector<NodeGraphNodeLink> links;

    Compiler compiler;
    const auto result = compiler.Compile(nodes, links);

    std::unordered_map<std::string, const LocalAssignStmt*> assignByVar;
    for (const auto& stmt : result.statements)
    {
        const auto* assign = dynamic_cast<const LocalAssignStmt*>(stmt.get());
        if (assign)
            assignByVar[assign->variableName] = assign;
    }

    ASSERT_EQ(assignByVar.size(), 2u);

    const auto intAssignIt = assignByVar.find("node_101");
    ASSERT_NE(intAssignIt, assignByVar.end());
    const auto* numberExpr = dynamic_cast<const NumberExpr*>(intAssignIt->second->value.get());
    ASSERT_NE(numberExpr, nullptr);
    EXPECT_EQ(numberExpr->value, "42");

    const auto boolAssignIt = assignByVar.find("node_202");
    ASSERT_NE(boolAssignIt, assignByVar.end());
    const auto* boolExpr = dynamic_cast<const BoolExpr*>(boolAssignIt->second->value.get());
    ASSERT_NE(boolExpr, nullptr);
    EXPECT_TRUE(boolExpr->value);

    EXPECT_TRUE(result.diagnostics.empty());
}

TEST(CompilerCompileExecChain, shouldEmitExecChainInsideFunctionBody)
{
    int nextInputPinId = 1000;
    int nextOutputPinId = 1;

    auto functionNode = std::make_unique<Function>(nextOutputPinId, std::vector<std::string>{"x"}, 10, "Function");
    auto printNode1 = std::make_unique<Print>(nextInputPinId, nextOutputPinId, 20, "Print1");
    auto printNode2 = std::make_unique<Print>(nextInputPinId, nextOutputPinId, 30, "Print2");

    NodeGraphNode* fn = functionNode.get();
    NodeGraphNode* p1 = printNode1.get();
    NodeGraphNode* p2 = printNode2.get();

    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    nodes.push_back(std::move(functionNode));
    nodes.push_back(std::move(printNode1));
    nodes.push_back(std::move(printNode2));

    std::vector<NodeGraphNodeLink> links;
    int nextLinkId = 1;

    links.emplace_back(nextLinkId++, fn->getExecOutput()->getId(), p1->getExecInput()->getId());
    links.emplace_back(nextLinkId++, p1->getExecOutput()->getId(), p2->getExecInput()->getId());

    links.emplace_back(nextLinkId++, fn->outputs()[0].getId(), p1->inputs()[0].getId());
    links.emplace_back(nextLinkId++, fn->outputs()[0].getId(), p2->inputs()[0].getId());

    Compiler compiler;
    const auto result = compiler.Compile(nodes, links);

    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_EQ(result.statements.size(), 1u);

    const auto* functionStmt = dynamic_cast<const FunctionStmt*>(result.statements[0].get());
    ASSERT_NE(functionStmt, nullptr);
    ASSERT_EQ(functionStmt->body.size(), 2u);

    EXPECT_NE(dynamic_cast<const PrintStmt*>(functionStmt->body[0].get()), nullptr);
    EXPECT_NE(dynamic_cast<const PrintStmt*>(functionStmt->body[1].get()), nullptr);
}

TEST(CompilerCompileExecChain, shouldNotDuplicateAlreadyEmittedExecNodes)
{
    int nextInputPinId = 2000;
    int nextOutputPinId = 1;

    auto functionNode = std::make_unique<Function>(nextOutputPinId, std::vector<std::string>{"x"}, 100, "Function");
    auto printNode = std::make_unique<Print>(nextInputPinId, nextOutputPinId, 200, "Print");
    auto returnNode = std::make_unique<Return>(nextInputPinId, 300, "Return");

    NodeGraphNode* fn = functionNode.get();
    NodeGraphNode* print = printNode.get();
    NodeGraphNode* ret = returnNode.get();

    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    nodes.push_back(std::move(functionNode));
    nodes.push_back(std::move(printNode));
    nodes.push_back(std::move(returnNode));

    std::vector<NodeGraphNodeLink> links;
    int nextLinkId = 1;

    links.emplace_back(nextLinkId++, fn->getExecOutput()->getId(), print->getExecInput()->getId());
    links.emplace_back(nextLinkId++, print->getExecOutput()->getId(), ret->getExecInput()->getId());

    links.emplace_back(nextLinkId++, fn->outputs()[0].getId(), print->inputs()[0].getId());
    links.emplace_back(nextLinkId++, fn->outputs()[0].getId(), ret->inputs()[0].getId());

    Compiler compiler;
    const auto result = compiler.Compile(nodes, links);

    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_EQ(result.statements.size(), 1u);

    const auto* functionStmt = dynamic_cast<const FunctionStmt*>(result.statements[0].get());
    ASSERT_NE(functionStmt, nullptr);
    ASSERT_EQ(functionStmt->body.size(), 2u);

    EXPECT_NE(dynamic_cast<const PrintStmt*>(functionStmt->body[0].get()), nullptr);
    EXPECT_NE(dynamic_cast<const ReturnStmt*>(functionStmt->body[1].get()), nullptr);
}

TEST(CompilerCompileResultStatements, shouldKeepTopLevelStatementsLinearAndNestedExecInsideFunctionBody)
{
    int nextInputPinId = 1000;
    int nextOutputPinId = 1;

    auto integerNode = std::make_unique<Integer>(nextOutputPinId, 101, "Integer");
    integerNode->outputs()[0].setValue("42");

    auto functionNode = std::make_unique<Function>(nextOutputPinId, std::vector<std::string>{"x"}, 10, "Function");
    auto printNode = std::make_unique<Print>(nextInputPinId, nextOutputPinId, 20, "Print");

    NodeGraphNode* fn = functionNode.get();
    NodeGraphNode* print = printNode.get();

    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    nodes.push_back(std::move(integerNode));
    nodes.push_back(std::move(functionNode));
    nodes.push_back(std::move(printNode));

    std::vector<NodeGraphNodeLink> links;
    int nextLinkId = 1;
    links.emplace_back(nextLinkId++, fn->getExecOutput()->getId(), print->getExecInput()->getId());
    links.emplace_back(nextLinkId++, fn->outputs()[0].getId(), print->inputs()[0].getId());

    Compiler compiler;
    const auto result = compiler.Compile(nodes, links);

    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_EQ(result.statements.size(), 2u);

    const auto* topLevelAssign = dynamic_cast<const LocalAssignStmt*>(result.statements[0].get());
    ASSERT_NE(topLevelAssign, nullptr);
    EXPECT_EQ(topLevelAssign->variableName, "node_101");

    const auto* topLevelAssignValue = dynamic_cast<const NumberExpr*>(topLevelAssign->value.get());
    ASSERT_NE(topLevelAssignValue, nullptr);
    EXPECT_EQ(topLevelAssignValue->value, "42");

    const auto* topLevelFunction = dynamic_cast<const FunctionStmt*>(result.statements[1].get());
    ASSERT_NE(topLevelFunction, nullptr);
    ASSERT_EQ(topLevelFunction->body.size(), 1u);

    const auto* printStmt = dynamic_cast<const PrintStmt*>(topLevelFunction->body[0].get());
    ASSERT_NE(printStmt, nullptr);

    const auto* printValue = dynamic_cast<const VariableExpr*>(printStmt->value.get());
    ASSERT_NE(printValue, nullptr);
    EXPECT_EQ(printValue->name, "x");
}
