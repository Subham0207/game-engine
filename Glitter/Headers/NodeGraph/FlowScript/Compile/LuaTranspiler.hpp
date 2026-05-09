//
// Created by subha on 03-05-2026.
//

#ifndef GLITTER_LUATRANSPILER_HPP
#define GLITTER_LUATRANSPILER_HPP

#include<string>
#include<vector>
#include<memory>
#include "Ast.hpp"

namespace Flowscript::Compile
{
    struct LuaTranspileOutput
    {
        std::string serializedNodePositions;
        std::string luaCode;
    };

    class LuaTranspiler
    {
        public:
            LuaTranspiler() = default;
            LuaTranspileOutput Transpile(const std::vector<std::unique_ptr<StatementAstNode>>& ast);
            std::string recurse(
                const AstNode* node
            );
            std::string recurseExpression(const ExpressionAstNode* node);

        private:
            std::string transpileFunctionNode(const FunctionStatementAstNode* node);
            std::string transpilePrintNode(const PrintStatementAstNode* node);
            std::string transpileReturnNode(const ReturnStatementAstNode* node);
            std::string transpileVariableDeclarationNode(const VariableDeclarationStatementAstNode* node);
            std::string transpileStatementWithTrailingExecution(const StatementAstNode* node, const std::string& currentStatement);
            std::string transpileExecutionFlowChildren(const StatementAstNode* node);
            static std::string resolveFunctionName(const FunctionStatementAstNode* node);
            static std::string resolveFunctionParameters(const FunctionStatementAstNode* node);
            std::string resolveVariableDeclarationValue(const VariableDeclarationStatementAstNode* node) const;
            std::string transpileBinaryExpression(const BinaryExpressionAstNode* node, const std::string& op);
            void appendNodePosition(const AstNode* node, std::string& serialized) const;
            void appendExpressionNodePosition(const ExpressionAstNode* node, std::string& serialized) const;
    };
}


#endif //GLITTER_LUATRANSPILER_HPP
