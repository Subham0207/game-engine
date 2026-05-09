#include <gtest/gtest.h>
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
    const std::string luaCode = luaTranspiler.Transpile(ast.programRoot);

    const std::string expectedLuaCode = "function foo(x)\n"
                                        "print()\n"
                                        "return\n"
                                        "end";
    ASSERT_EQ(luaCode, expectedLuaCode);
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
    const std::string luaCode = luaTranspiler.Transpile(ast);

    const std::string expectedLuaCode = "function sum(a,b)\n"
                                        "return (1 + 2)\n"
                                        "end";
    ASSERT_EQ(luaCode, expectedLuaCode);
}

TEST(LuaTranspiler, shouldEmitFunctionWithoutParametersWhenNoneProvided)
{
    std::vector<std::unique_ptr<Flowscript::Compile::StatementAstNode>> ast;

    auto functionNode = std::make_unique<FunctionStatementAstNode>();
    functionNode->functionName = "ping";
    ast.push_back(std::move(functionNode));

    LuaTranspiler luaTranspiler;
    const std::string luaCode = luaTranspiler.Transpile(ast);
    ASSERT_EQ(luaCode, "function ping()\nend");
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
    const std::string luaCode = luaTranspiler.Transpile(ast);

    const std::string expectedLuaCode = "function stopAtReturn()\n"
                                        "return\n"
                                        "end";
    ASSERT_EQ(luaCode, expectedLuaCode);
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
    const std::string luaCode = luaTranspiler.Transpile(ast);

    const std::string expectedLuaCode = "function useVar()\n"
                                        "local score = 10\n"
                                        "print(score)\n"
                                        "end";
    ASSERT_EQ(luaCode, expectedLuaCode);
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
    const std::string luaCode = luaTranspiler.Transpile(ast);

    const std::string expectedLuaCode = "function ops()\n"
                                        "return (((6 * 2) < (9 / 3)) == (7 % 4))\n"
                                        "end";
    ASSERT_EQ(luaCode, expectedLuaCode);
}
