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
            std::string Transpile(const std::vector<std::unique_ptr<AstNode>>& ast);
            std::string recurse(
                const AstNode* node
            );
            std::string recurseInputChildren(const AstNode* node);

        private:
            std::string transpileFunctionNode(const AstNode* node);
            std::string transpilePrintNode(const AstNode* node);
            std::string transpileReturnNode(const AstNode* node);
            std::string transpileUnknownNode(const AstNode* node);
            std::string transpileStatementWithTrailingExecution(const AstNode* node, const std::string& currentStatement);
            std::string transpileExecutionFlowChildren(const AstNode* node);
            std::string resolveFunctionName(const AstNode* node) const;
            std::string resolveFunctionParameters(const AstNode* node) const;
    };
}


#endif //GLITTER_LUATRANSPILER_HPP
