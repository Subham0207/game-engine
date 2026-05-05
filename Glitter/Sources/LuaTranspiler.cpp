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
                //what kind of input child this is (Binary operator)
                // we use that info to fill out a syntax template.
            }
        }

        if (!node->outputExecutionFlows.empty())
        {
            for (auto& outputExec: node->outputExecutionFlows)
            {
                recurse(outputExec.get());
                //what kind of statement it is. We use that info to fill out a syntax template.
            }
        }
    }

    std::string LuaTranspiler::recurseInputChildren(const AstNode* node)
    {
        if (node->inputDataChildrens.empty())
            return "";

        std::string expr = "";
        for (auto& inputChild: node->inputDataChildrens)
        {
            expr += recurseInputChildren(inputChild.get());
        }
        return expr;
    }
}
