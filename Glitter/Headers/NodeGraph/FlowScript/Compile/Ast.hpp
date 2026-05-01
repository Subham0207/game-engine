//
// Created by subha on 28-04-2026.
//

#ifndef GLITTER_AST_HPP
#define GLITTER_AST_HPP
#include <vector>
#include <memory>
#include "NodeGraph/Components/NodeGraphNodeLink.hpp"
#include <string>

class NodeGraphNode;
namespace Flowscript::Compile
{
    struct AstNode
    {
        std::string type;
        std::vector<std::unique_ptr<AstNode>> children;// Let's start with placing exec flow nodes and data flow nodes in same children array.
    };

    class Ast
    {
        public:
            Ast(
                const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
                const std::vector<NodeGraphNodeLink>& links
            );

            void recurse(
                AstNode* current,
                NodeGraphNode* currentNode,
                const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
                const std::vector<NodeGraphNodeLink>& links
            );

            std::unique_ptr<AstNode> recurseForDataValue(
                int currAttrId,
                const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
                const std::vector<NodeGraphNodeLink>& links
            );

        int inDegree(
            NodeGraphNode* currentNode,
            const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
            const std::vector<NodeGraphNodeLink>& links);

            std::vector<std::unique_ptr<AstNode>> programRoot;
    };

}


#endif //GLITTER_AST_HPP
