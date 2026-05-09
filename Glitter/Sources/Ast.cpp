//
// Created by subha on 28-04-2026.
//

#include "../Headers/NodeGraph/FlowScript/Compile/Ast.hpp"
#include <NodeGraph/Components/NodeGraphNode.hpp>
#include <NodeGraph/Components/NodeGraphNodes/Variables/GetVariable.hpp>
#include <NodeGraph/Components/NodeGraphNodes/Variables/VariableDeclaration.hpp>
#include <NodeGraph/Components/NodeGraphNodes/Keywords/Function.hpp>
#include <stdexcept>

namespace Flowscript::Compile
{
    namespace
    {
        static std::string resolveFunctionName(NodeGraphNode* node)
        {
            const auto* functionNode = dynamic_cast<const NodeGraphComponents::Node::Keywords::Function*>(node);
            if (!functionNode || functionNode->functionName.empty())
                throw std::runtime_error("Function node requires a function name");

            return functionNode->functionName;
        }

        static std::vector<std::string> collectFunctionParams(NodeGraphNode* node)
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

        static Flowscript::Compile::VariableValueType resolveVariableValueType(const std::string& rawType)
        {
            if (rawType == "boolean" || rawType == "Boolean")
                return Flowscript::Compile::VariableValueType::Boolean;
            if (rawType == "string" || rawType == "String")
                return Flowscript::Compile::VariableValueType::String;
            if (rawType == "table" || rawType == "Table")
                return Flowscript::Compile::VariableValueType::Table;
            if (rawType == "array" || rawType == "Array")
                return Flowscript::Compile::VariableValueType::Array;
            if (rawType == "record" || rawType == "Record")
                return Flowscript::Compile::VariableValueType::Record;

            return Flowscript::Compile::VariableValueType::Number;
        }

        static std::string firstOutputValue(NodeGraphNode* node)
        {
            if (!node || node->outputs().empty())
                return "";
            return node->outputs()[0].getValueBuff();
        }

        static std::unique_ptr<StatementAstNode> makeStatementNode(NodeGraphNode* node)
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
                case NodeTypes::VariableDeclaration:
                {
                    auto stmt = std::make_unique<VariableDeclarationStatementAstNode>();
                    const auto* declNode = dynamic_cast<const NodeGraphComponents::Node::Variables::VariableDeclaration*>(node);
                    if (!declNode || declNode->variableName.empty())
                        throw std::runtime_error("Variable declaration requires a variable name");
                    stmt->name = declNode->variableName;
                    stmt->valueType = resolveVariableValueType(declNode->declaredType);
                    stmt->value = declNode->value;
                    return stmt;
                }
                default:
                    return nullptr;
            }
        }

        static std::unique_ptr<ExpressionAstNode> makeExpressionNode(NodeGraphNode* node)
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
                case NodeTypes::GetVariable:
                {
                    auto expr = std::make_unique<GetVariableExpressionAstNode>();
                    const auto* getVarNode = dynamic_cast<const NodeGraphComponents::Node::Variables::GetVariable*>(node);
                    if (!getVarNode || getVarNode->variableName.empty())
                        throw std::runtime_error("GetVariable node requires a variable name");
                    expr->name = getVarNode->variableName;
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

        static int findLinkStartAttrForEndAttr(
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

        static NodeGraphNode* findNodeByOutputAttr(
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

        static int countIncomingExecLinks(
            NodeGraphNode* currentNode,
            const std::vector<NodeGraphNodeLink>& links
        )
        {
            int inDegree = 0;
            if (!currentNode || !currentNode->hasExecInput())
                return inDegree;

            for (const auto& link: links)
            {
                if (link.endAttr() == currentNode->getExecInput()->getId())
                    ++inDegree;
            }

            return inDegree;
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
            if (node->hasExecOutput() && countIncomingExecLinks(node.get(), links) == 0)
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
            return;
        }
        else if (auto* varDeclStmt = dynamic_cast<VariableDeclarationStatementAstNode*>(current))
        {
            const auto* varDeclNode = dynamic_cast<const NodeGraphComponents::Node::Variables::VariableDeclaration*>(currentNode);
            if (varDeclNode)
                varDeclStmt->value = varDeclNode->value;
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
}
