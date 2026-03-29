//
// Created by subha on 28-03-2026.
//

#include "../Headers/NodeGraph/FlowScript/Compile/Compiler.hpp"

#include <algorithm>

#include "NodeGraph/FlowScript/Compile/Helpers.hpp"

namespace Flowscript::Compile
{
    namespace
    {
        std::unique_ptr<NumberExpr> MakeDefaultNumberExpr()
        {
            auto expr = std::make_unique<NumberExpr>();
            expr->value = "0";
            return expr;
        }
    }

    CompileResult Compiler::Compile(const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
                                    const std::vector<NodeGraphNodeLink>& links)
    {
        diagnostics_.clear();
        activeExprNodes_.clear();

        graph_ = BuildGraphIndex(nodes, links);
        ValidateGraph(links);

        CompileResult result;

        // Emit generic object prelude at top-level.
        for (auto* node : graph_.allNodes)
        {
            if (!node || !isGenericTypeNodeName(node->name()))
                continue;

            const std::string objectName = getGenericObjectName(node->name());
            if (objectName.empty())
                continue;

            // `t` is runtime context injected as function parameter in transition conditions.
            // Do not emit local table/literal assignments for it.
            if (objectName == "t")
                continue;

            auto tableStmt = std::make_unique<LocalTableStmt>();
            tableStmt->variableName = objectName;
            result.statements.push_back(std::move(tableStmt));

            for (auto& output : node->outputs())
            {
                if (output.getType() != NodeAttributeType::FIELD)
                    continue;

                std::unique_ptr<Expr> valueExpr;
                std::string literal = trimCopy(output.getValueBuff());
                if (literal.empty())
                    literal = "0";

                if (isBooleanLiteral(literal))
                {
                    auto expr = std::make_unique<BoolExpr>();
                    expr->value = parseBooleanOrDefault(literal.c_str());
                    valueExpr = std::move(expr);
                }
                else if (isNumberLiteral(literal))
                {
                    auto expr = std::make_unique<NumberExpr>();
                    expr->value = parseNumberOrDefault(literal.c_str());
                    valueExpr = std::move(expr);
                }
                else
                {
                    diagnostics_.push_back("Ignoring unsupported GenericType field literal on '" + objectName + "." + output.getName() + "'");
                    continue;
                }

                auto assign = std::make_unique<LocalAssignStmt>();
                assign->variableName = objectName + "." + output.getName();
                assign->value = std::move(valueExpr);
                result.statements.push_back(std::move(assign));
            }
        }

        // Emit pure data expression nodes as local assignments in data dependency order.
        const auto ordered = TopoSortDataNodes(graph_);
        for (auto* node : ordered)
        {
            if (!node || node->hasExecInput() || node->hasExecOutput())
                continue;

            if (!IsExpressionNode(node->type()))
                continue;

            auto localAssign = std::make_unique<LocalAssignStmt>();
            localAssign->variableName = NodeVarName(*node);
            localAssign->value = CompileExpr(node);
            result.statements.push_back(std::move(localAssign));
        }

        std::unordered_set<int> activeExecNodes;
        std::unordered_set<int> emittedExecNodes;

        // Start from execution roots first.
        for (auto* node : graph_.allNodes)
        {
            if (!node || !(node->hasExecInput() || node->hasExecOutput()))
                continue;

            const auto indegreeIt = graph_.execIndegree.find(node);
            if (indegreeIt != graph_.execIndegree.end() && indegreeIt->second == 0)
            {
                auto chain = CompileExecChain(node, activeExecNodes, emittedExecNodes);
                for (auto& stmt : chain)
                    result.statements.push_back(std::move(stmt));
            }
        }

        // Emit any disconnected exec components as well.
        for (auto* node : graph_.allNodes)
        {
            if (!node || !(node->hasExecInput() || node->hasExecOutput()))
                continue;
            if (emittedExecNodes.find(node->id()) != emittedExecNodes.end())
                continue;

            auto chain = CompileExecChain(node, activeExecNodes, emittedExecNodes);
            for (auto& stmt : chain)
                result.statements.push_back(std::move(stmt));
        }

        result.diagnostics = diagnostics_;
        return result;
    }

    void Compiler::ValidateGraph(const std::vector<NodeGraphNodeLink>& links)
    {
        for (const auto& link : links)
        {
            const auto* start = FindPinInfo(graph_, link.startAttr());
            const auto* end = FindPinInfo(graph_, link.endAttr());
            if (!start || !end)
            {
                diagnostics_.push_back("Link references unknown pin id(s): " + std::to_string(link.id()));
                continue;
            }

            if (start->isInput)
                diagnostics_.push_back("Link starts from an input pin: " + std::to_string(link.id()));
            if (!end->isInput)
                diagnostics_.push_back("Link ends at a non-input pin: " + std::to_string(link.id()));

            const bool execLink = start->type == NodeAttributeType::ExecutionFlowOutPin
                               || end->type == NodeAttributeType::ExecutionFlowInPin;
            if (execLink)
            {
                if (!(start->type == NodeAttributeType::ExecutionFlowOutPin
                    && end->type == NodeAttributeType::ExecutionFlowInPin))
                {
                    diagnostics_.push_back("Invalid execution link types for link: " + std::to_string(link.id()));
                }
            }
            else if (!(end->type == NodeAttributeType::PIN
                    && (start->type == NodeAttributeType::PIN || start->type == NodeAttributeType::FIELD)))
            {
                diagnostics_.push_back("Invalid data link types for link: " + std::to_string(link.id()));
            }
        }

        for (auto* node : graph_.allNodes)
        {
            if (!node)
                continue;

            const int requiredInputs = RequiredInputCount(node->type());
            for (int i = 0; i < requiredInputs && i < static_cast<int>(node->inputs().size()); ++i)
            {
                const int inputId = node->inputs()[i].getId();
                if (graph_.dataSourcePinByInputPin.find(inputId) == graph_.dataSourcePinByInputPin.end())
                {
                    diagnostics_.push_back("Missing required input on node '" + node->name() + "' (id=" + std::to_string(node->id()) + ")");
                }
            }
        }
    }

    std::string Compiler::NodeVarName(const NodeGraphNode& node)
    {
        return "node_" + std::to_string(node.id());
    }

    bool Compiler::IsExpressionNode(const NodeTypes type)
    {
        return type == NodeTypes::Integer
            || type == NodeTypes::Boolean
            || type == NodeTypes::Add
            || type == NodeTypes::Subtract
            || type == NodeTypes::GreaterThan
            || type == NodeTypes::EqualsTo
            || type == NodeTypes::NotEqualsTo;
    }

    int Compiler::RequiredInputCount(const NodeTypes type)
    {
        if (type == NodeTypes::Add || type == NodeTypes::Subtract || type == NodeTypes::GreaterThan
            || type == NodeTypes::EqualsTo || type == NodeTypes::NotEqualsTo)
            return 2;
        if (type == NodeTypes::Print || type == NodeTypes::Return)
            return 1;
        return 0;
    }

    std::unique_ptr<Expr> Compiler::CompileExpr(NodeGraphNode* node)
    {
        if (!node)
            return MakeDefaultNumberExpr();

        if (activeExprNodes_.find(node->id()) != activeExprNodes_.end())
        {
            diagnostics_.push_back("Data cycle detected while compiling expression node id=" + std::to_string(node->id()));
            return MakeDefaultNumberExpr();
        }

        activeExprNodes_.insert(node->id());

        std::unique_ptr<Expr> result;
        switch (node->type())
        {
            case NodeTypes::Integer:
            {
                auto expr = std::make_unique<NumberExpr>();
                if (!node->outputs().empty())
                    expr->value = parseNumberOrDefault(node->outputs()[0].getValueBuff());
                else
                    expr->value = "0";
                result = std::move(expr);
                break;
            }
            case NodeTypes::Boolean:
            {
                auto expr = std::make_unique<BoolExpr>();
                expr->value = !node->outputs().empty() && parseBooleanOrDefault(node->outputs()[0].getValueBuff());
                result = std::move(expr);
                break;
            }
            case NodeTypes::Add:
            case NodeTypes::Subtract:
            case NodeTypes::GreaterThan:
            case NodeTypes::EqualsTo:
            case NodeTypes::NotEqualsTo:
            {
                auto expr = std::make_unique<BinaryExpr>();
                if (node->type() == NodeTypes::Add)
                    expr->op = "+";
                else if (node->type() == NodeTypes::Subtract)
                    expr->op = "-";
                else if (node->type() == NodeTypes::GreaterThan)
                    expr->op = ">";
                else if (node->type() == NodeTypes::EqualsTo)
                    expr->op = "==";
                else
                    expr->op = "~=";

                expr->lhs = CompileInputExpr(*node, 0);
                expr->rhs = CompileInputExpr(*node, 1);
                result = std::move(expr);
                break;
            }
            default:
            {
                diagnostics_.push_back("Unsupported expression node: " + node->name());
                result = MakeDefaultNumberExpr();
                break;
            }
        }

        activeExprNodes_.erase(node->id());
        return result;
    }

    std::unique_ptr<Expr> Compiler::CompileInputExpr(NodeGraphNode& node, const int inputIndex)
    {
        if (inputIndex < 0 || inputIndex >= static_cast<int>(node.inputs().size()))
            return MakeDefaultNumberExpr();

        const auto& input = node.inputs()[inputIndex];
        const auto* sourcePin = GetDataSourcePinInfo(graph_, input.getId());

        if (!sourcePin || !sourcePin->node)
        {
            diagnostics_.push_back("Input not connected on node '" + node.name() + "' (id=" + std::to_string(node.id()) + ")");
            return MakeDefaultNumberExpr();
        }

        NodeGraphNode* sourceNode = sourcePin->node;

        if (isGenericTypeNodeName(sourceNode->name()) && sourcePin->index >= 0)
        {
            const std::string objectName = getGenericObjectName(sourceNode->name());
            const auto& outputs = sourceNode->outputs();
            if (!objectName.empty() && sourcePin->index < static_cast<int>(outputs.size()))
            {
                auto expr = std::make_unique<VariableExpr>();
                expr->name = objectName + "." + outputs[sourcePin->index].getName();
                return expr;
            }
        }

        if (sourceNode->type() == NodeTypes::Function && sourcePin->index >= 0)
        {
            const auto& outputs = sourceNode->outputs();
            if (sourcePin->index < static_cast<int>(outputs.size()))
            {
                auto expr = std::make_unique<VariableExpr>();
                const std::string pinName = trimCopy(outputs[sourcePin->index].getName());
                expr->name = pinName.empty() ? ("arg" + std::to_string(sourcePin->index)) : pinName;
                return expr;
            }
        }

        auto expr = std::make_unique<VariableExpr>();
        expr->name = NodeVarName(*sourceNode);
        return expr;
    }

    std::unique_ptr<Stmt> Compiler::CompileExecStatement(NodeGraphNode* node,
                                                         std::unordered_set<int>& activeExecNodes,
                                                         std::unordered_set<int>& emittedExecNodes)
    {
        if (!node)
            return nullptr;

        if (activeExecNodes.find(node->id()) != activeExecNodes.end())
        {
            diagnostics_.push_back("Execution cycle detected at node id=" + std::to_string(node->id()));
            return nullptr;
        }

        activeExecNodes.insert(node->id());
        emittedExecNodes.insert(node->id());

        std::unique_ptr<Stmt> stmt;

        switch (node->type())
        {
            case NodeTypes::Function:
            {
                auto fn = std::make_unique<FunctionStmt>();
                fn->name = NodeVarName(*node);

                for (size_t i = 0; i < node->outputs().size(); ++i)
                {
                    std::string paramName = trimCopy(node->outputs()[i].getName());
                    if (paramName.empty())
                        paramName = "arg" + std::to_string(i);
                    fn->parameters.push_back(std::move(paramName));
                }

                const auto next = GetExecNextNodes(graph_, node);
                for (auto* child : next)
                {
                    auto bodyChain = CompileExecChain(child, activeExecNodes, emittedExecNodes);
                    for (auto& s : bodyChain)
                        fn->body.push_back(std::move(s));
                }

                stmt = std::move(fn);
                break;
            }
            case NodeTypes::Print:
            {
                auto printStmt = std::make_unique<PrintStmt>();
                printStmt->value = CompileInputExpr(*node, 0);
                stmt = std::move(printStmt);
                break;
            }
            case NodeTypes::Return:
            {
                auto returnStmt = std::make_unique<ReturnStmt>();
                returnStmt->value = CompileInputExpr(*node, 0);
                stmt = std::move(returnStmt);
                break;
            }
            case NodeTypes::Add:
            case NodeTypes::Subtract:
            case NodeTypes::GreaterThan:
            case NodeTypes::EqualsTo:
            case NodeTypes::NotEqualsTo:
            {
                auto assign = std::make_unique<LocalAssignStmt>();
                assign->variableName = NodeVarName(*node);
                assign->value = CompileExpr(node);
                stmt = std::move(assign);
                break;
            }
            default:
            {
                diagnostics_.push_back("Unsupported exec node in chain: " + node->name());
                break;
            }
        }

        activeExecNodes.erase(node->id());
        return stmt;
    }

    std::vector<std::unique_ptr<Stmt>> Compiler::CompileExecChain(NodeGraphNode* start,
                                                                   std::unordered_set<int>& activeExecNodes,
                                                                   std::unordered_set<int>& emittedExecNodes)
    {
        std::vector<std::unique_ptr<Stmt>> chain;
        if (!start)
            return chain;

        if (emittedExecNodes.find(start->id()) != emittedExecNodes.end())
            return chain;

        auto stmt = CompileExecStatement(start, activeExecNodes, emittedExecNodes);
        if (stmt)
            chain.push_back(std::move(stmt));

        if (start->type() == NodeTypes::Return)
            return chain;

        const auto nextNodes = GetExecNextNodes(graph_, start);
        for (auto* next : nextNodes)
        {
            auto child = CompileExecChain(next, activeExecNodes, emittedExecNodes);
            for (auto& s : child)
                chain.push_back(std::move(s));
        }

        return chain;
    }
}
