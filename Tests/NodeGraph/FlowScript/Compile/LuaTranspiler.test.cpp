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
using Flowscript::Compile::Ast;
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

    //TODO: add simple functions for test to attach exec flow nodes.
    links.emplace_back(nextLinkId++, fn->getExecOutputId(), print->getExecInputId());
    links.emplace_back(nextLinkId++, print->getExecOutputId(), ret->getExecInputId());

    const Ast ast(nodes, links);
    //----------------

    /*
        function foo()
            print('Hello world')
        end
    */

}