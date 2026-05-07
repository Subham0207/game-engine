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
    class LuaTranspiler
    {
        public:
            LuaTranspiler() = default;
            std::string Transpile(const std::vector<std::unique_ptr<StatementAstNode>>& ast);
            std::string recurse(
                const AstNode* node
            );
            std::string recurseExpression(const ExpressionAstNode* node);

        private:
            std::string transpileFunctionNode(const FunctionStatementAstNode* node);
            std::string transpilePrintNode(const PrintStatementAstNode* node);
            std::string transpileReturnNode(const ReturnStatementAstNode* node);
            std::string transpileStatementWithTrailingExecution(const StatementAstNode* node, const std::string& currentStatement);
            std::string transpileExecutionFlowChildren(const StatementAstNode* node);
            std::string resolveFunctionName(const FunctionStatementAstNode* node) const;
            std::string resolveFunctionParameters(const FunctionStatementAstNode* node) const;
            std::string transpileBinaryExpression(const BinaryExpressionAstNode* node, const std::string& op);
    };
}


#endif //GLITTER_LUATRANSPILER_HPP
