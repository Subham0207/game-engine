//
// Created by subha on 28-04-2026.
//

#include "../Headers/NodeGraph/FlowScript/Compile/Ast.hpp"
#include <NodeGraph/Components/NodeGraphNode.hpp>

Flowscript::Compile::Ast::Ast(
    const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
    const std::vector<NodeGraphNodeLink>& links
)
{
    // using Nodes and links we build the AST
    //Start from the node where no inputExec exists.
    std::vector<NodeGraphNode*> rootNodes;
    for (const auto& node: nodes)
    {
        if (node->getExecInput() == nullptr)
        {
            rootNodes.push_back(node.get());
        }
    }

    for (const auto& node: rootNodes)
    {
        //Construct Tree for this rootNode...
        auto root = std::make_unique<AstNode>();
        programRoot.emplace_back(std::move(root));
        recurse(root.get(), node, nodes, links);
    }
}

void Flowscript::Compile::Ast::recurse(
    AstNode* current,
    const NodeGraphNode* currentNode,
    const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
    const std::vector<NodeGraphNodeLink>& links
)
{
    //look for a link that starts from this node's execOutput
    for (const auto& link: links)
    {
        //using the link find which is the end node exec flow attaches to.
        auto endExecNodeAttr = -1;
        if (link.startAttr() == currentNode->getExecOutput()->getId())
        {
            endExecNodeAttr = link.endAttr();
            break;
        }

        //find the nodeGraphNode for this endNodeAttr
        for (const auto& node: nodes)
        {
            if (node->getExecInput()->getId() == endExecNodeAttr)
            {
                auto child = new AstNode();
                current->children.push_back(child);
                recurse(child, node.get(), nodes, links);
            }
        }
    }
}
