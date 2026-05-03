//
// Created by subha on 28-04-2026.
//

#include "../Headers/NodeGraph/FlowScript/Compile/Ast.hpp"
#include <NodeGraph/Components/NodeGraphNode.hpp>

namespace Flowscript::Compile
{
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
