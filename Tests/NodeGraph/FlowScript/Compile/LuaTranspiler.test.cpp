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
//
// Created by subha on 05-05-2026.
//
TEST(LuaTranspiler, shouldTranspileAstToLua)
{
    //Create AST-----------

    // function node -> print node -> return node.
    int nextLinkId = 1;

    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    std::vector<NodeGraphNodeLink> links;

    auto nodeGraphIdAllocator = new NodeGraphIdAllocator();
    auto nodeGraphFactory = new NodeGraphNodeFactory(nodeGraphIdAllocator);

    auto fn = nodeGraphFactory->addFunctionNode(nodes, std::vector<std::string>{"x"});
    auto print = nodeGraphFactory->addNode(nodes, NodeTypes::Print);
    auto ret = nodeGraphFactory->addNode(nodes, NodeTypes::Return);

    links.emplace_back(nextLinkId++, fn->getExecOutputId(), print->getExecInputId());
    links.emplace_back(nextLinkId++, print->getExecOutputId(), ret->getExecInputId());

    const Ast ast(nodes, links);
    //----------------

    /*
        function foo()
            print('Hello world')
        end
    */

    // Lua can understand oneliners. There are some cases where it cannot.
    // So semicolon is used to separate statements.
    // Therefore, newlines or semicolons are essentials.
    // Lua does not care about indentations.
    std::string expectedLuaCode =  "function foo(x)\n"
                            "print('Hello world')\n"
                            "return\n"
                            "end";

    LuaTranspiler luaTranspiler;
    std::string luaCode = luaTranspiler.Transpile(ast.programRoot);
    ASSERT_EQ(luaCode, expectedLuaCode);
}

TEST(LuaTranspiler, shouldUseFunctionNameFromAstNode)
{
    std::vector<std::unique_ptr<Flowscript::Compile::StatementAstNode>> ast;

    auto functionNode = std::make_unique<Flowscript::Compile::FunctionStatementAstNode>();
    functionNode->functionName = "sum";
    functionNode->parameters = {"a", "b"};

    auto returnNode = std::make_unique<Flowscript::Compile::ReturnStatementAstNode>();

    auto addNode = std::make_unique<Flowscript::Compile::AddExpressionAstNode>();

    auto lhs = std::make_unique<Flowscript::Compile::IntegerLiteralExpressionAstNode>();
    lhs->value = "1";

    auto rhs = std::make_unique<Flowscript::Compile::IntegerLiteralExpressionAstNode>();
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
