//
// Created by subha on 28-04-2026.
//

#ifndef GLITTER_AST_HPP
#define GLITTER_AST_HPP

#include <vector>
#include <memory>
#include <string>
#include "NodeGraph/Components/NodeGraphNodeLink.hpp"
#include "AstNodes/AstNodes.hpp"

class NodeGraphNode;

namespace Flowscript::Compile
{
    class Ast
    {
    public:
        Ast(
            const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
            const std::vector<NodeGraphNodeLink>& links
        );

        void recurse(
            StatementAstNode* current,
            NodeGraphNode* currentNode,
            const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
            const std::vector<NodeGraphNodeLink>& links
        );

        std::unique_ptr<ExpressionAstNode> recurseForDataValue(
            int currAttrId,
            const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
            const std::vector<NodeGraphNodeLink>& links
        );


        std::vector<std::unique_ptr<StatementAstNode>> programRoot;
    };

}


#endif //GLITTER_AST_HPP
