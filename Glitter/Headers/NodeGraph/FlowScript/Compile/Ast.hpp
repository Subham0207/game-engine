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
    enum class AstNodeKind
    {
        Unknown,
        Statement,
        Expression
    };

    enum class AstStatementOpcode
    {
        None,
        Function,
        Print,
        Return,
        Unknown
    };

    enum class AstExpressionOpcode
    {
        None,
        IntegerLiteral,
        BooleanLiteral,
        Add,
        Subtract,
        EqualsTo,
        GreaterThan,
        NotEqualsTo,
        Unknown
    };

    struct AstNode
    {
        //children of AstNode... BinaryNode, local Declaration, function.
        std::string type;
        // Function identifier; kept separate from variableName (which stores function params today).
        std::string functionName;
        std::string variableName;
        std::string value;
        AstNodeKind kind = AstNodeKind::Unknown;
        AstStatementOpcode statementOpcode = AstStatementOpcode::None;
        AstExpressionOpcode expressionOpcode = AstExpressionOpcode::None;

        // inputDataChildrens, outputDataChildrens, inputExecutionFlow, outputExecutionFlows (serialized sequentially).
        // This is because there is order of serialization into Lua code for these properties. we cannot have them sequentially under one array.
        std::vector<std::unique_ptr<AstNode>> inputDataChildrens;
        std::vector<std::unique_ptr<AstNode>> outputExecutionFlows;
        // Below are backwards link...
        std::vector<std::unique_ptr<AstNode>> outputDataChildrens;
        std::unique_ptr<AstNode> inputExecutionFlow;
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
