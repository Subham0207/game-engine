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
            void recurse(
                const AstNode* node
            );
            void recurseInputChildren(const AstNode* node);
    };
}


#endif //GLITTER_LUATRANSPILER_HPP
