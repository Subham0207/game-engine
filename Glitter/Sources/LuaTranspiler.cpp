//
// Created by subha on 03-05-2026.
//

#include "../Headers/NodeGraph/FlowScript/Compile/LuaTranspiler.hpp"

namespace Flowscript::Compile
{
    std::string LuaTranspiler::Transpile(const std::vector<std::unique_ptr<AstNode>>& ast)
    {
        for (auto& node: ast)
        {
            recurse(node.get());
        }
    }

    void LuaTranspiler::recurse(const AstNode* node)
    {
        if (!node->inputDataChildrens.empty())
        {
            for (auto& inputChild: node->inputDataChildrens)
            {
                recurseInputChildren(inputChild.get());
            }
        }
    }

    void LuaTranspiler::recurseInputChildren(const AstNode* node)
    {
        if (node->inputDataChildrens.empty())
            return;
        for (auto& inputChild: node->inputDataChildrens)
        {
            recurseInputChildren(inputChild.get());
        }
    }
}
