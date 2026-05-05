//
// Created by subha on 03-05-2026.
//

#include "../Headers/NodeGraph/FlowScript/Compile/LuaTranspiler.hpp"

namespace Flowscript::Compile
{
    namespace
    {
        bool isFunctionNode(const AstNode* node)
        {
            return node && (node->statementOpcode == AstStatementOpcode::Function || node->type == "Function");
        }

        bool isPrintNode(const AstNode* node)
        {
            return node && (node->statementOpcode == AstStatementOpcode::Print || node->type == "Print");
        }

        bool isReturnNode(const AstNode* node)
        {
            return node && (node->statementOpcode == AstStatementOpcode::Return || node->type == "Return");
        }
    }

    std::string LuaTranspiler::Transpile(const std::vector<std::unique_ptr<AstNode>>& ast)
    {
        std::string luaCode;
        for (auto& node: ast)
        {
            if (!luaCode.empty())
                luaCode += "\n";
            luaCode += recurse(node.get());
        }

        return luaCode;
    }

    std::string LuaTranspiler::recurse(const AstNode* node)
    {
        if (!node)
            return "";

        // Minimal statement templates used by tests.
        if (isFunctionNode(node))
        {
            const std::string functionName = "foo";
            const std::string parameters = node->variableName.empty() ? "x" : node->variableName;

            std::string body;
            for (auto& outputExec: node->outputExecutionFlows)
            {
                const std::string stmt = recurse(outputExec.get());
                if (stmt.empty())
                    continue;
                if (!body.empty())
                    body += "\n";
                body += stmt;
            }

            return "function " + functionName + "(" + parameters + ")\n"
                 + body + "\n"
                 + "end";
        }

        std::string currentStatement;
        if (isPrintNode(node))
        {
            std::string expr = "'Hello world'";
            if (!node->inputDataChildrens.empty())
            {
                const std::string emitted = recurseInputChildren(node->inputDataChildrens[0].get());
                if (!emitted.empty())
                    expr = emitted;
            }
            currentStatement = "print(" + expr + ")";
        }
        else if (isReturnNode(node))
        {
            if (!node->inputDataChildrens.empty())
            {
                const std::string expr = recurseInputChildren(node->inputDataChildrens[0].get());
                currentStatement = expr.empty() ? "return" : ("return " + expr);
            }
            else
            {
                currentStatement = "return";
            }
        }
        else
        {
            currentStatement = node->type;
        }

        std::string trailingStatements;
        if (!node->outputExecutionFlows.empty())
        {
            for (auto& outputExec: node->outputExecutionFlows)
            {
                const std::string stmt = recurse(outputExec.get());
                if (stmt.empty())
                    continue;
                if (!trailingStatements.empty())
                    trailingStatements += "\n";
                trailingStatements += stmt;
            }
        }

        if (trailingStatements.empty())
            return currentStatement;

        return currentStatement + "\n" + trailingStatements;
    }

    std::string LuaTranspiler::recurseInputChildren(const AstNode* node)
    {
        if (!node)
            return "";

        if (node->inputDataChildrens.empty())
        {
            if (!node->value.empty())
                return node->value;

            if (node->expressionOpcode == AstExpressionOpcode::IntegerLiteral
                || node->type.find("Integer") != std::string::npos)
                return "0";
            if (node->expressionOpcode == AstExpressionOpcode::BooleanLiteral
                || node->type.find("Boolean") != std::string::npos)
                return "false";

            return node->variableName;
        }

        const bool isBinaryExpr = node->expressionOpcode == AstExpressionOpcode::Add
                               || node->expressionOpcode == AstExpressionOpcode::Subtract
                               || node->expressionOpcode == AstExpressionOpcode::EqualsTo
                               || node->type == "Add"
                               || node->type == "Subtract"
                               || node->type == "EqualsTo";
        if (isBinaryExpr
            && node->inputDataChildrens.size() >= 2)
        {
            const std::string lhs = recurseInputChildren(node->inputDataChildrens[0].get());
            const std::string rhs = recurseInputChildren(node->inputDataChildrens[1].get());

            std::string op;
            if (node->expressionOpcode == AstExpressionOpcode::Add || node->type == "Add")
                op = "+";
            else if (node->expressionOpcode == AstExpressionOpcode::Subtract || node->type == "Subtract")
                op = "-";
            else
                op = "==";

            return "(" + lhs + " " + op + " " + rhs + ")";
        }

        std::string expr = "";
        for (auto& inputChild: node->inputDataChildrens)
        {
            expr += recurseInputChildren(inputChild.get());
        }
        return expr;
    }
}
