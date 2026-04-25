//
// Created by subha on 28-03-2026.
//

#ifndef GLITTER_COMPILER_HPP
#define GLITTER_COMPILER_HPP
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "GraphIndexLookup.hpp"
#include "IntermediateRepresentation/Expression.hpp"
#include "IntermediateRepresentation/Statement.hpp"

namespace Flowscript::Compile
{
    struct CompileResult
    {
        std::vector<std::unique_ptr<Stmt>> statements;
        std::vector<std::string> diagnostics;
    };

    class Compiler
    {
    public:
        CompileResult Compile(const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
                              const std::vector<NodeGraphNodeLink>& links);

    private:
        GraphIndex graph_;
        std::vector<std::string> diagnostics_;
        std::unordered_set<int> activeExprNodes_;

        void ValidateGraph(const std::vector<NodeGraphNodeLink>& links);
        static std::string NodeVarName(const NodeGraphNode& node);
        static bool IsExpressionNode(NodeTypes type);
        static int RequiredInputCount(NodeTypes type);
        void EmitDataAssignments(CompileResult& result);
        std::string ResolveGenericObjectSymbol(NodeGraphNode& genericNode) const;

        std::unique_ptr<Expr> CompileExpr(NodeGraphNode* node);
        std::unique_ptr<Expr> CompileInputExpr(NodeGraphNode& node, int inputIndex);

        std::unique_ptr<Stmt> CompileExecStatement(NodeGraphNode* node,
                                                   std::unordered_set<int>& activeExecNodes,
                                                   std::unordered_set<int>& emittedExecNodes);
        std::vector<std::unique_ptr<Stmt>> CompileExecChain(NodeGraphNode* current,
                                                            std::unordered_set<int>& activeExecNodes,
                                                            std::unordered_set<int>& emittedExecNodes);
    };
}


#endif //GLITTER_COMPILER_HPP




