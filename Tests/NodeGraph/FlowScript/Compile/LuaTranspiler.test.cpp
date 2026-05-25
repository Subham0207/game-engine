#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
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
#include "NodeGraph/FlowScript/Compile/LuaTranspiler.hpp"
#include "NodeGraph/FlowScript/VisualScriptJsonSerializer.hpp"

using NodeGraphComponents::Node::Integer;
using NodeGraphComponents::Node::Add;
using NodeGraphComponents::Node::EqualsTo;
using NodeGraphComponents::Node::Keywords::Function;
using NodeGraphComponents::Node::Keywords::Print;
using NodeGraphComponents::Node::Keywords::Return;
using NodeGraphComponents::NodeGraphNodeFactory;
using NodeGraphComponents::NodeGraphIdAllocator;
using Flowscript::Compile::Ast;
using Flowscript::Compile::LuaTranspiler;
using Flowscript::Compile::FunctionStatementAstNode;
using Flowscript::Compile::ReturnStatementAstNode;
using Flowscript::Compile::PrintStatementAstNode;
using Flowscript::Compile::VariableDeclarationStatementAstNode;
using Flowscript::Compile::VariableValueType;
using Flowscript::Compile::GetVariableExpressionAstNode;
using Flowscript::Compile::LessThanExpressionAstNode;
using Flowscript::Compile::MultiplyExpressionAstNode;
using Flowscript::Compile::DivideExpressionAstNode;
using Flowscript::Compile::ModuloExpressionAstNode;
using Flowscript::Compile::IntegerLiteralExpressionAstNode;
using Flowscript::Compile::GreaterThanExpressionAstNode;
using Flowscript::Compile::NotEqualsToExpressionAstNode;
using Flowscript::Serialization::VisualScriptJsonSerializer;

TEST(LuaTranspiler, shouldTranspileAstToLua)
{
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

    links.emplace_back(nextLinkId++, fn->getExecOutputId(), print->getExecInputId());
    links.emplace_back(nextLinkId++, print->getExecOutputId(), ret->getExecInputId());

    Ast ast(nodes, links);
    auto* functionNode = dynamic_cast<FunctionStatementAstNode*>(ast.programRoot[0].get());
    ASSERT_NE(functionNode, nullptr);
    ASSERT_EQ(functionNode->functionName, "foo");

    LuaTranspiler luaTranspiler;
    const auto output = luaTranspiler.Transpile(ast.programRoot);

    const std::string expectedLuaCode = "function foo(x)\n"
                                        "print()\n"
                                        "return\n"
                                        "end";
    ASSERT_EQ(output.luaCode, expectedLuaCode);
    ASSERT_FALSE(output.serializedNodePositions.empty());
}

TEST(LuaTranspiler, shouldThrowWhenFunctionNameMissing)
{
    std::vector<std::unique_ptr<Flowscript::Compile::StatementAstNode>> ast;
    auto functionNode = std::make_unique<FunctionStatementAstNode>();
    ast.push_back(std::move(functionNode));

    LuaTranspiler luaTranspiler;
    EXPECT_THROW(luaTranspiler.Transpile(ast), std::runtime_error);
}

TEST(LuaTranspiler, shouldUseFunctionNameFromAstNode)
{
    std::vector<std::unique_ptr<Flowscript::Compile::StatementAstNode>> ast;

    auto functionNode = std::make_unique<FunctionStatementAstNode>();
    functionNode->functionName = "sum";
    functionNode->parameters = {"a", "b"};

    auto returnNode = std::make_unique<ReturnStatementAstNode>();
    auto addNode = std::make_unique<Flowscript::Compile::AddExpressionAstNode>();

    auto lhs = std::make_unique<IntegerLiteralExpressionAstNode>();
    lhs->value = "1";

    auto rhs = std::make_unique<IntegerLiteralExpressionAstNode>();
    rhs->value = "2";

    addNode->lhs = std::move(lhs);
    addNode->rhs = std::move(rhs);
    returnNode->expression = std::move(addNode);
    functionNode->outputExecutionFlows.push_back(std::move(returnNode));
    ast.push_back(std::move(functionNode));

    LuaTranspiler luaTranspiler;
    const auto output = luaTranspiler.Transpile(ast);

    const std::string expectedLuaCode = "function sum(a,b)\n"
                                        "return (1 + 2)\n"
                                        "end";
    ASSERT_EQ(output.luaCode, expectedLuaCode);
}

TEST(LuaTranspiler, shouldEmitFunctionWithoutParametersWhenNoneProvided)
{
    std::vector<std::unique_ptr<Flowscript::Compile::StatementAstNode>> ast;

    auto functionNode = std::make_unique<FunctionStatementAstNode>();
    functionNode->functionName = "ping";
    ast.push_back(std::move(functionNode));

    LuaTranspiler luaTranspiler;
    const auto output = luaTranspiler.Transpile(ast);
    ASSERT_EQ(output.luaCode, "function ping()\nend");
}

TEST(LuaTranspiler, shouldTerminateExecutionChainAtReturn)
{
    std::vector<std::unique_ptr<Flowscript::Compile::StatementAstNode>> ast;

    auto functionNode = std::make_unique<FunctionStatementAstNode>();
    functionNode->functionName = "stopAtReturn";

    auto returnNode = std::make_unique<ReturnStatementAstNode>();
    auto printAfterReturn = std::make_unique<PrintStatementAstNode>();
    returnNode->outputExecutionFlows.push_back(std::move(printAfterReturn));
    functionNode->outputExecutionFlows.push_back(std::move(returnNode));

    ast.push_back(std::move(functionNode));

    LuaTranspiler luaTranspiler;
    const auto output = luaTranspiler.Transpile(ast);

    const std::string expectedLuaCode = "function stopAtReturn()\n"
                                        "return\n"
                                        "end";
    ASSERT_EQ(output.luaCode, expectedLuaCode);
}

TEST(LuaTranspiler, shouldTranspileVariableDeclarationAndGetVariable)
{
    std::vector<std::unique_ptr<Flowscript::Compile::StatementAstNode>> ast;

    auto functionNode = std::make_unique<FunctionStatementAstNode>();
    functionNode->functionName = "useVar";

    auto varDecl = std::make_unique<VariableDeclarationStatementAstNode>();
    varDecl->name = "score";
    varDecl->valueType = VariableValueType::Number;
    varDecl->value = "10";

    auto printNode = std::make_unique<PrintStatementAstNode>();
    auto getVar = std::make_unique<GetVariableExpressionAstNode>();
    getVar->name = "score";
    printNode->expression = std::move(getVar);

    varDecl->outputExecutionFlows.push_back(std::move(printNode));
    functionNode->outputExecutionFlows.push_back(std::move(varDecl));
    ast.push_back(std::move(functionNode));

    LuaTranspiler luaTranspiler;
    const auto output = luaTranspiler.Transpile(ast);

    const std::string expectedLuaCode = "function useVar()\n"
                                        "local score = 10\n"
                                        "print(score)\n"
                                        "end";
    ASSERT_EQ(output.luaCode, expectedLuaCode);
}

TEST(LuaTranspiler, shouldThrowWhenVariableDeclarationNameMissing)
{
    std::vector<std::unique_ptr<Flowscript::Compile::StatementAstNode>> ast;

    auto functionNode = std::make_unique<FunctionStatementAstNode>();
    functionNode->functionName = "invalidVarDecl";

    auto varDecl = std::make_unique<VariableDeclarationStatementAstNode>();
    varDecl->valueType = VariableValueType::String;

    functionNode->outputExecutionFlows.push_back(std::move(varDecl));
    ast.push_back(std::move(functionNode));

    LuaTranspiler luaTranspiler;
    EXPECT_THROW(luaTranspiler.Transpile(ast), std::runtime_error);
}

TEST(LuaTranspiler, shouldTranspileAdditionalBinaryExpressions)
{
    std::vector<std::unique_ptr<Flowscript::Compile::StatementAstNode>> ast;

    auto functionNode = std::make_unique<FunctionStatementAstNode>();
    functionNode->functionName = "ops";

    auto returnNode = std::make_unique<ReturnStatementAstNode>();

    auto lessThanNode = std::make_unique<LessThanExpressionAstNode>();

    auto multiplyNode = std::make_unique<MultiplyExpressionAstNode>();
    auto divideNode = std::make_unique<DivideExpressionAstNode>();
    auto moduloNode = std::make_unique<ModuloExpressionAstNode>();

    auto one = std::make_unique<IntegerLiteralExpressionAstNode>();
    one->value = "6";
    auto two = std::make_unique<IntegerLiteralExpressionAstNode>();
    two->value = "2";
    multiplyNode->lhs = std::move(one);
    multiplyNode->rhs = std::move(two);

    auto three = std::make_unique<IntegerLiteralExpressionAstNode>();
    three->value = "9";
    auto four = std::make_unique<IntegerLiteralExpressionAstNode>();
    four->value = "3";
    divideNode->lhs = std::move(three);
    divideNode->rhs = std::move(four);

    auto five = std::make_unique<IntegerLiteralExpressionAstNode>();
    five->value = "7";
    auto six = std::make_unique<IntegerLiteralExpressionAstNode>();
    six->value = "4";
    moduloNode->lhs = std::move(five);
    moduloNode->rhs = std::move(six);

    lessThanNode->lhs = std::move(multiplyNode);
    lessThanNode->rhs = std::move(divideNode);

    auto equalsNode = std::make_unique<Flowscript::Compile::EqualsToExpressionAstNode>();
    equalsNode->lhs = std::move(lessThanNode);
    equalsNode->rhs = std::move(moduloNode);

    returnNode->expression = std::move(equalsNode);
    functionNode->outputExecutionFlows.push_back(std::move(returnNode));
    ast.push_back(std::move(functionNode));

    LuaTranspiler luaTranspiler;
    const auto output = luaTranspiler.Transpile(ast);

    const std::string expectedLuaCode = "function ops()\n"
                                        "return (((6 * 2) < (9 / 3)) == (7 % 4))\n"
                                        "end";
    ASSERT_EQ(output.luaCode, expectedLuaCode);
}

TEST(LuaTranspiler, shouldSerializeAstNodePositions)
{
    std::vector<std::unique_ptr<Flowscript::Compile::StatementAstNode>> ast;

    auto functionNode = std::make_unique<FunctionStatementAstNode>();
    functionNode->functionName = "pos";
    functionNode->x = 10.0f;
    functionNode->y = 20.0f;

    auto printNode = std::make_unique<PrintStatementAstNode>();
    printNode->x = 30.0f;
    printNode->y = 40.0f;

    auto intExpr = std::make_unique<IntegerLiteralExpressionAstNode>();
    intExpr->x = 50.0f;
    intExpr->y = 60.0f;
    intExpr->value = "1";
    printNode->expression = std::move(intExpr);

    functionNode->outputExecutionFlows.push_back(std::move(printNode));
    ast.push_back(std::move(functionNode));

    LuaTranspiler luaTranspiler;
    const auto output = luaTranspiler.Transpile(ast);

    ASSERT_EQ(output.serializedNodePositions, "10.00,20.00;30.00,40.00;50.00,60.00");
}

TEST(LuaTranspiler, shouldTranspileGreaterThanExpressionInsideReturn)
{
    std::vector<std::unique_ptr<Flowscript::Compile::StatementAstNode>> ast;

    auto functionNode = std::make_unique<FunctionStatementAstNode>();
    functionNode->functionName = "isGreater";

    auto returnNode = std::make_unique<ReturnStatementAstNode>();
    auto greaterThanNode = std::make_unique<GreaterThanExpressionAstNode>();

    auto lhs = std::make_unique<IntegerLiteralExpressionAstNode>();
    lhs->value = "10";
    auto rhs = std::make_unique<IntegerLiteralExpressionAstNode>();
    rhs->value = "5";

    greaterThanNode->lhs = std::move(lhs);
    greaterThanNode->rhs = std::move(rhs);
    returnNode->expression = std::move(greaterThanNode);

    functionNode->outputExecutionFlows.push_back(std::move(returnNode));
    ast.push_back(std::move(functionNode));

    LuaTranspiler luaTranspiler;
    const auto output = luaTranspiler.Transpile(ast);

    const std::string expectedLuaCode = "function isGreater()\n"
                                        "return (10 > 5)\n"
                                        "end";
    ASSERT_EQ(output.luaCode, expectedLuaCode);
}

TEST(LuaTranspiler, shouldTranspileNotEqualsExpressionInsideReturn)
{
    std::vector<std::unique_ptr<Flowscript::Compile::StatementAstNode>> ast;

    auto functionNode = std::make_unique<FunctionStatementAstNode>();
    functionNode->functionName = "isDifferent";

    auto returnNode = std::make_unique<ReturnStatementAstNode>();
    auto notEqualsNode = std::make_unique<NotEqualsToExpressionAstNode>();

    auto lhs = std::make_unique<IntegerLiteralExpressionAstNode>();
    lhs->value = "7";
    auto rhs = std::make_unique<IntegerLiteralExpressionAstNode>();
    rhs->value = "9";

    notEqualsNode->lhs = std::move(lhs);
    notEqualsNode->rhs = std::move(rhs);
    returnNode->expression = std::move(notEqualsNode);

    functionNode->outputExecutionFlows.push_back(std::move(returnNode));
    ast.push_back(std::move(functionNode));

    LuaTranspiler luaTranspiler;
    const auto output = luaTranspiler.Transpile(ast);

    const std::string expectedLuaCode = "function isDifferent()\n"
                                        "return (7 ~= 9)\n"
                                        "end";
    ASSERT_EQ(output.luaCode, expectedLuaCode);
}

TEST(LuaTranspiler, shouldTranspileReturnBooleanFromDeserializedFlowScriptGraph)
{
    const std::string flowScriptJson = R"JSON({"formatVersion":1,"nodes":[{"id":0,"type":14,"name":"Function","x":3.87E2,"y":3.68E2,"inputs":[],"outputs":[{"id":4000,"name":"t","type":1}],"execOutput":{"id":4001,"name":"","type":3},"metadata":{"functionName":"condition"}},{"id":1,"type":16,"name":"Return","x":1.195E3,"y":3.78E2,"inputs":[{"id":3000,"name":"ReturnInputPin","type":1}],"outputs":[],"execInput":{"id":3001,"name":"","type":2}},{"id":2,"type":10,"name":"Boolean","x":1.067E3,"y":6.59E2,"inputs":[],"outputs":[{"id":4002,"name":"Value","type":0,"value":"true"}]},{"id":3,"type":11,"name":"GenericType(t)","x":6.43E2,"y":5.42E2,"inputs":[{"id":3002,"name":"Object","type":1}],"outputs":[{"id":4003,"name":"isGrounded","type":1},{"id":4004,"name":"dodgeStart","type":1},{"id":4005,"name":"punchStarted","type":1},{"id":4006,"name":"sidekickStarted","type":1},{"id":4007,"name":"locomotionX","type":1},{"id":4008,"name":"locomotionY","type":1}]}],"links":[{"id":2000,"startAttr":4001,"endAttr":3001},{"id":2001,"startAttr":4002,"endAttr":3000},{"id":2002,"startAttr":4000,"endAttr":3002}]})JSON";

    const std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "LuaTranspiler_shouldTranspileReturnBooleanFromDeserializedFlowScriptGraph.flowscript";
    {
        std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
        ASSERT_TRUE(out.is_open());
        out << flowScriptJson;
    }

    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    std::vector<NodeGraphNodeLink> links;
    std::string error;
    const bool loadOk = VisualScriptJsonSerializer::DeserializeFromFile(tempPath, nodes, links, &error);

    std::error_code ec;
    std::filesystem::remove(tempPath, ec);

    ASSERT_TRUE(loadOk) << error;

    Ast ast(nodes, links);
    LuaTranspiler luaTranspiler;
    const auto output = luaTranspiler.Transpile(ast.programRoot);

    const std::string expectedLuaCode = "function condition(t)\n"
                                        "return true\n"
                                        "end";
    ASSERT_EQ(output.luaCode, expectedLuaCode);
}

TEST(LuaTranspiler, shouldTranspileGreaterThanUsingGenericMemberFromDeserializedFlowScriptGraph)
{
    const std::string flowScriptJson = R"JSON({"formatVersion":1,"nodes":[{"id":0,"type":14,"name":"Function","x":4.32E2,"y":4.13E2,"inputs":[],"outputs":[{"id":4000,"name":"t","type":1}],"execOutput":{"id":4001,"name":"","type":3},"metadata":{"functionName":"condition"}},{"id":1,"type":16,"name":"Return","x":1.236E3,"y":4.61E2,"inputs":[{"id":3000,"name":"ReturnInputPin","type":1}],"outputs":[],"execInput":{"id":3001,"name":"","type":2}},{"id":2,"type":10,"name":"Boolean","x":8.79E2,"y":6.57E2,"inputs":[],"outputs":[{"id":4002,"name":"Value","type":0,"value":"true"}]},{"id":3,"type":11,"name":"GenericType(t)","x":6.88E2,"y":5.99E2,"inputs":[{"id":3002,"name":"Object","type":1}],"outputs":[{"id":4003,"name":"isGrounded","type":1},{"id":4004,"name":"dodgeStart","type":1},{"id":4005,"name":"punchStarted","type":1},{"id":4006,"name":"sidekickStarted","type":1},{"id":4007,"name":"locomotionX","type":1},{"id":4008,"name":"locomotionY","type":1}]},{"id":4,"type":6,"name":"GreaterThan","x":9.76E2,"y":4.4E2,"inputs":[{"id":3003,"name":"A","type":1},{"id":3004,"name":"B","type":1}],"outputs":[{"id":4009,"name":"isAGreaterThanB","type":1}],"execInput":{"id":3005,"name":"","type":2},"execOutput":{"id":4010,"name":"","type":3}}],"links":[{"id":2002,"startAttr":4000,"endAttr":3002},{"id":2003,"startAttr":4001,"endAttr":3005},{"id":2004,"startAttr":4010,"endAttr":3001},{"id":2005,"startAttr":4003,"endAttr":3003},{"id":2006,"startAttr":4002,"endAttr":3004},{"id":2007,"startAttr":4009,"endAttr":3000}]})JSON";

    const std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "LuaTranspiler_shouldTranspileGreaterThanUsingGenericMemberFromDeserializedFlowScriptGraph.flowscript";
    {
        std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
        ASSERT_TRUE(out.is_open());
        out << flowScriptJson;
    }

    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    std::vector<NodeGraphNodeLink> links;
    std::string error;
    const bool loadOk = VisualScriptJsonSerializer::DeserializeFromFile(tempPath, nodes, links, &error);

    std::error_code ec;
    std::filesystem::remove(tempPath, ec);

    ASSERT_TRUE(loadOk) << error;

    Ast ast(nodes, links);
    LuaTranspiler luaTranspiler;
    const auto output = luaTranspiler.Transpile(ast.programRoot);

    const std::string expectedLuaCode = "function condition(t)\n"
                                        "return (t.isGrounded > true)\n"
                                        "end";
    ASSERT_EQ(output.luaCode, expectedLuaCode);
}

