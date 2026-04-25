#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "NodeGraph/Components/NodeGraphNode.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Boolean.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Integer.hpp"
#include "NodeGraph/FlowScript/Compile/Compiler.hpp"

using Flowscript::Compile::BoolExpr;
using Flowscript::Compile::Compiler;
using Flowscript::Compile::LocalAssignStmt;
using Flowscript::Compile::NumberExpr;
using NodeGraphComponents::Node::Boolean;
using NodeGraphComponents::Node::Integer;

TEST(CompilerEmitPureDataExpressionAssignments, EmitsDataExpressionAssignments)
{
    // Example input graph for this test:
    //   Integer(id=101, value="42")   Boolean(id=202, value="true")
    //   (no links, no exec flow)
    // Expected compile output includes:
    //   local node_101 = 42
    //   local node_202 = true

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

    // Keep only local assignments so we can assert by variable name.
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
