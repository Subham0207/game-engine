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
    std::vector<std::unique_ptr<Flowscript::Compile::AstNode>> ast;

    auto functionNode = std::make_unique<Flowscript::Compile::AstNode>();
    functionNode->type = "Function";
    functionNode->statementOpcode = Flowscript::Compile::AstStatementOpcode::Function;
    functionNode->functionName = "sum";
    functionNode->variableName = "a,b";

    auto returnNode = std::make_unique<Flowscript::Compile::AstNode>();
    returnNode->type = "Return";
    returnNode->statementOpcode = Flowscript::Compile::AstStatementOpcode::Return;

    auto addNode = std::make_unique<Flowscript::Compile::AstNode>();
    addNode->type = "Add";
    addNode->expressionOpcode = Flowscript::Compile::AstExpressionOpcode::Add;

    auto lhs = std::make_unique<Flowscript::Compile::AstNode>();
    lhs->type = "Integer";
    lhs->expressionOpcode = Flowscript::Compile::AstExpressionOpcode::IntegerLiteral;
    lhs->value = "1";

    auto rhs = std::make_unique<Flowscript::Compile::AstNode>();
    rhs->type = "Integer";
    rhs->expressionOpcode = Flowscript::Compile::AstExpressionOpcode::IntegerLiteral;
    rhs->value = "2";

    addNode->inputDataChildrens.push_back(std::move(lhs));
    addNode->inputDataChildrens.push_back(std::move(rhs));
    returnNode->inputDataChildrens.push_back(std::move(addNode));
    functionNode->outputExecutionFlows.push_back(std::move(returnNode));
    ast.push_back(std::move(functionNode));

    LuaTranspiler luaTranspiler;
    const std::string luaCode = luaTranspiler.Transpile(ast);

    const std::string expectedLuaCode = "function sum(a,b)\n"
                                        "return (1 + 2)\n"
                                        "end";
    ASSERT_EQ(luaCode, expectedLuaCode);
}
