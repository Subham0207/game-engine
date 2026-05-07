//
// Created by subha on 28-04-2026.
//

#include "../Headers/NodeGraph/FlowScript/Compile/Ast.hpp"
#include <NodeGraph/Components/NodeGraphNode.hpp>

namespace Flowscript::Compile
{
    namespace
    {
        std::string resolveFunctionName(NodeGraphNode* node)
        {
            if (!node)
                return "";

            const std::string& candidate = node->name();
            if (candidate.empty() || candidate == "Function")
                return "";

            return candidate;
        }

        std::vector<std::string> collectFunctionParams(NodeGraphNode* node)
        {
            std::vector<std::string> params;
            if (!node)
                return params;

            for (const auto& output: node->outputs())
            {
                const std::string& name = output.getName();
                if (!name.empty())
                    params.push_back(name);
            }

            return params;
        }

        std::string firstOutputValue(NodeGraphNode* node)
        {
            if (!node || node->outputs().empty())
                return "";
            return node->outputs()[0].getValueBuff();
        }

        std::unique_ptr<StatementAstNode> makeStatementNode(NodeGraphNode* node)
        {
            if (!node)
                return nullptr;

            switch (node->type())
            {
                case NodeTypes::Function:
                {
                    auto stmt = std::make_unique<FunctionStatementAstNode>();
                    stmt->functionName = resolveFunctionName(node);
                    stmt->parameters = collectFunctionParams(node);
                    return stmt;
                }
                case NodeTypes::Print:
                    return std::make_unique<PrintStatementAstNode>();
                case NodeTypes::Return:
                    return std::make_unique<ReturnStatementAstNode>();
                default:
                    return nullptr;
            }
        }

        std::unique_ptr<ExpressionAstNode> makeExpressionNode(NodeGraphNode* node)
        {
            if (!node)
                return nullptr;

            switch (node->type())
            {
                case NodeTypes::Integer:
                {
                    auto expr = std::make_unique<IntegerLiteralExpressionAstNode>();
                    expr->value = firstOutputValue(node);
                    return expr;
                }
                case NodeTypes::Boolean:
                {
                    auto expr = std::make_unique<BooleanLiteralExpressionAstNode>();
                    expr->value = firstOutputValue(node);
                    return expr;
                }
                case NodeTypes::Add:
                    return std::make_unique<AddExpressionAstNode>();
                case NodeTypes::Subtract:
                    return std::make_unique<SubtractExpressionAstNode>();
                case NodeTypes::EqualsTo:
                    return std::make_unique<EqualsToExpressionAstNode>();
                case NodeTypes::GreaterThan:
                    return std::make_unique<GreaterThanExpressionAstNode>();
                case NodeTypes::NotEqualsTo:
                    return std::make_unique<NotEqualsToExpressionAstNode>();
                default:
                    return nullptr;
            }
        }

        int findLinkStartAttrForEndAttr(
            int endAttr,
            const std::vector<NodeGraphNodeLink>& links
        )
        {
            for (const auto& link: links)
            {
                if (link.endAttr() == endAttr)
                    return link.startAttr();
            }

            return -1;
        }

        NodeGraphNode* findNodeByOutputAttr(
            const int outputAttr,
            const std::vector<std::unique_ptr<NodeGraphNode>>& nodes
        )
        {
            if (outputAttr == -1)
                return nullptr;

            for (const auto& node: nodes)
            {
                for (const auto& output: node->outputs())
                {
                    if (output.getId() == outputAttr)
                        return node.get();
                }
            }

            return nullptr;
        }
    }

    Ast::Ast(
        const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
        const std::vector<NodeGraphNodeLink>& links
    )
    {
        // using Nodes and links we build the AST
        // Start from exec root nodes (no exec input, has exec output).
        std::vector<NodeGraphNode*> rootNodes;
        for (const auto& node: nodes)
        {
            if (node->hasExecOutput() && inDegree(node.get(), nodes, links) == 0)
            {
                rootNodes.push_back(node.get());
            }
        }

        for (const auto& node: rootNodes)
        {
            auto root = makeStatementNode(node);
            if (!root)
                continue;
            programRoot.push_back(std::move(root));
            recurse(programRoot.back().get(), node, nodes, links);
        }
    }

    void Ast::recurse(
        StatementAstNode* current,
        NodeGraphNode* currentNode,
        const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
        const std::vector<NodeGraphNodeLink>& links
    )
    {
        if (current == nullptr || currentNode == nullptr || !currentNode->hasExecOutput())
        {
            return;
        }

        if (auto* printStmt = dynamic_cast<PrintStatementAstNode*>(current))
        {
            if (!currentNode->inputs().empty())
                printStmt->expression = recurseForDataValue(currentNode->inputs()[0].getId(), nodes, links);
        }
        else if (auto* returnStmt = dynamic_cast<ReturnStatementAstNode*>(current))
        {
            if (!currentNode->inputs().empty())
                returnStmt->expression = recurseForDataValue(currentNode->inputs()[0].getId(), nodes, links);
        }

        auto endExecNodeAttr = -1;
        for (const auto& link: links)
        {
            if (currentNode->hasExecOutput() && link.startAttr() == currentNode->getExecOutput()->getId())
            {
                endExecNodeAttr = link.endAttr();
                break;
            }
        }

        for (const auto& node: nodes)
        {
            if (node->hasExecInput() && node->getExecInput()->getId() == endExecNodeAttr)
            {
                auto child = makeStatementNode(node.get());
                if (!child)
                    continue;
                current->outputExecutionFlows.push_back(std::move(child));
                recurse(
                    current->outputExecutionFlows.back().get()
                    , node.get()
                    , nodes
                    , links);
            }
        }
    }

    std::unique_ptr<ExpressionAstNode> Ast::recurseForDataValue(
        int currAttrId,
        const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
        const std::vector<NodeGraphNodeLink>& links
    )
    {
        const int startAttr = findLinkStartAttrForEndAttr(currAttrId, links);
        if (startAttr == -1)
            return nullptr;

        NodeGraphNode* sourceNode = findNodeByOutputAttr(startAttr, nodes);
        if (!sourceNode)
            return nullptr;

        auto expressionNode = makeExpressionNode(sourceNode);
        if (!expressionNode)
            return nullptr;

        if (auto* binary = dynamic_cast<BinaryExpressionAstNode*>(expressionNode.get()))
        {
            if (sourceNode->inputs().size() >= 2)
            {
                binary->lhs = recurseForDataValue(sourceNode->inputs()[0].getId(), nodes, links);
                binary->rhs = recurseForDataValue(sourceNode->inputs()[1].getId(), nodes, links);
            }
        }

        return expressionNode;
    }

    int Ast::inDegree(
        NodeGraphNode* currentNode,
        const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
        const std::vector<NodeGraphNodeLink>& links)
    {
        int inDegree = 0;

        if (!currentNode->hasExecInput())
            return inDegree;

        for (const auto& link: links)
        {
            //if this link and node are attached.
            if (link.endAttr() == currentNode->getExecInput()->getId())
            {
                inDegree++;
            }
        }

        return inDegree;
    }
}
