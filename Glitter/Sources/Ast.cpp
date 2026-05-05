//
// Created by subha on 28-04-2026.
//

#include "../Headers/NodeGraph/FlowScript/Compile/Ast.hpp"
#include <NodeGraph/Components/NodeGraphNode.hpp>

namespace Flowscript::Compile
{
    namespace
    {
        AstStatementOpcode mapStatementOpcode(const NodeTypes type)
        {
            switch (type)
            {
                case NodeTypes::Function: return AstStatementOpcode::Function;
                case NodeTypes::Print: return AstStatementOpcode::Print;
                case NodeTypes::Return: return AstStatementOpcode::Return;
                default: return AstStatementOpcode::Unknown;
            }
        }

        AstExpressionOpcode mapExpressionOpcode(const NodeTypes type)
        {
            switch (type)
            {
                case NodeTypes::Integer: return AstExpressionOpcode::IntegerLiteral;
                case NodeTypes::Boolean: return AstExpressionOpcode::BooleanLiteral;
                case NodeTypes::Add: return AstExpressionOpcode::Add;
                case NodeTypes::Subtract: return AstExpressionOpcode::Subtract;
                case NodeTypes::EqualsTo: return AstExpressionOpcode::EqualsTo;
                case NodeTypes::GreaterThan: return AstExpressionOpcode::GreaterThan;
                case NodeTypes::NotEqualsTo: return AstExpressionOpcode::NotEqualsTo;
                default: return AstExpressionOpcode::Unknown;
            }
        }

        std::string firstOutputValue(NodeGraphNode* node)
        {
            if (!node || node->outputs().empty())
                return "";
            return node->outputs()[0].getValueBuff();
        }

        std::string joinFunctionParams(NodeGraphNode* node)
        {
            if (!node)
                return "";

            std::string params;
            for (size_t i = 0; i < node->outputs().size(); ++i)
            {
                const std::string name = node->outputs()[i].getName();
                if (name.empty())
                    continue;
                if (!params.empty())
                    params += ",";
                params += name;
            }

            return params;
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
            // Construct tree for this root node.
            auto root = std::make_unique<AstNode>();
            root->type = node->name();
            root->kind = AstNodeKind::Statement;
            root->statementOpcode = mapStatementOpcode(node->type());
            root->variableName = joinFunctionParams(node);
            programRoot.push_back(std::move(root));
            recurse(programRoot.back().get(), node, nodes, links);
        }
    }

    void Ast::recurse(
        AstNode* current,
        NodeGraphNode* currentNode,
        const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
        const std::vector<NodeGraphNodeLink>& links
    )
    {
        if (current == nullptr || currentNode == nullptr || !currentNode->hasExecOutput())
        {
            return;
        }

        // For any data value attributes on this node. resolve that tree.
        for (auto& input: currentNode->inputs())
        {
            //build a tree from this attribute...
            auto ast = recurseForDataValue(input.getId(), nodes, links);
            if (ast)
                current->inputDataChildrens.push_back(std::move(ast));
        }

        current->kind = AstNodeKind::Statement;
        current->statementOpcode = mapStatementOpcode(currentNode->type());
        if (current->statementOpcode == AstStatementOpcode::Function)
            current->variableName = joinFunctionParams(currentNode);

        //look for a link that starts from this node's execOutput
        auto endExecNodeAttr = -1;
        for (const auto& link: links)
        {
            //using the link find which is the end node exec flow attaches to.
            if (currentNode->hasExecOutput() && link.startAttr() == currentNode->getExecOutput()->getId())
            {
                endExecNodeAttr = link.endAttr();
                break;
            }
        }

        //find the nodeGraphNode for this endNodeAttr
        for (const auto& node: nodes)
        {
            if (node->hasExecInput() && node->getExecInput()->getId() == endExecNodeAttr)
            {
                auto child = std::make_unique<AstNode>();
                child->type = node->name();
                child->kind = AstNodeKind::Statement;
                child->statementOpcode = mapStatementOpcode(node->type());
                if (child->statementOpcode == AstStatementOpcode::Function)
                    child->variableName = joinFunctionParams(node.get());
                current->outputExecutionFlows.push_back(std::move(child));
                recurse(
                    current->outputExecutionFlows.back().get()
                    , node.get()
                    , nodes
                    , links);
            }
        }
    }

    std::unique_ptr<AstNode> Ast::recurseForDataValue(
        int currAttrId,
        const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
        const std::vector<NodeGraphNodeLink>& links)
    {
        //find the connection
        //search which link ends with currAttrId.
        int startAttr = -1;
        for (const auto& link: links)
        {
            if (link.endAttr() == currAttrId)
            {
                startAttr = link.startAttr();
            }
        }

        if (startAttr == -1)
            return nullptr;

        //find the node where this startAttr is there.
        NodeGraphNode* nodeWithOutStartAttr = nullptr;
        for (const auto& node: nodes)
        {
            for (const auto& output: node->outputs())
            {
                if (output.getId() == startAttr)
                {
                    nodeWithOutStartAttr = node.get();
                    break;
                }
            }
            if (nodeWithOutStartAttr)
                break;
        }

        if (!nodeWithOutStartAttr) return nullptr;
        //create new ast node... And recursive Asts will be added to its children
        auto currentAstNode = std::make_unique<AstNode>();
        currentAstNode->type = nodeWithOutStartAttr->name();
        currentAstNode->kind = AstNodeKind::Expression;
        currentAstNode->expressionOpcode = mapExpressionOpcode(nodeWithOutStartAttr->type());
        currentAstNode->value = firstOutputValue(nodeWithOutStartAttr);

        //resolve all input attributes of this node recursively.
        for (auto& input: nodeWithOutStartAttr->inputs())
        {
            auto ast = recurseForDataValue(input.getId(), nodes, links);
            if (ast)
                currentAstNode->inputDataChildrens.push_back(std::move(ast));
        }

        return currentAstNode;
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
