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

        if (isFunctionNode(node))
            return transpileFunctionNode(node);
        if (isPrintNode(node))
            return transpilePrintNode(node);
        if (isReturnNode(node))
            return transpileReturnNode(node);
        return transpileUnknownNode(node);
    }

    std::string LuaTranspiler::transpileFunctionNode(const AstNode* node)
    {
        const std::string functionName = resolveFunctionName(node);
        const std::string parameters = resolveFunctionParameters(node);
        const std::string body = transpileExecutionFlowChildren(node);

        if (body.empty())
            return "function " + functionName + "(" + parameters + ")\nend";

        return "function " + functionName + "(" + parameters + ")\n"
             + body + "\n"
             + "end";
    }

    std::string LuaTranspiler::transpilePrintNode(const AstNode* node)
    {
        std::string expr = "'Hello world'";
        if (!node->inputDataChildrens.empty())
        {
            const std::string emitted = recurseInputChildren(node->inputDataChildrens[0].get());
            if (!emitted.empty())
                expr = emitted;
        }

        return transpileStatementWithTrailingExecution(node, "print(" + expr + ")");
    }

    std::string LuaTranspiler::transpileReturnNode(const AstNode* node)
    {
        std::string currentStatement = "return";
        if (!node->inputDataChildrens.empty())
        {
            const std::string expr = recurseInputChildren(node->inputDataChildrens[0].get());
            currentStatement = expr.empty() ? "return" : ("return " + expr);
        }

        return transpileStatementWithTrailingExecution(node, currentStatement);
    }

    std::string LuaTranspiler::transpileUnknownNode(const AstNode* node)
    {
        return transpileStatementWithTrailingExecution(node, node->type);
    }

    std::string LuaTranspiler::transpileStatementWithTrailingExecution(
        const AstNode* node,
        const std::string& currentStatement
    )
    {
        const std::string trailingStatements = transpileExecutionFlowChildren(node);
        if (trailingStatements.empty())
            return currentStatement;

        return currentStatement + "\n" + trailingStatements;
    }

    std::string LuaTranspiler::transpileExecutionFlowChildren(const AstNode* node)
    {
        if (!node)
            return "";

        std::string statements;
        for (const auto& outputExec: node->outputExecutionFlows)
        {
            const std::string stmt = recurse(outputExec.get());
            if (stmt.empty())
                continue;
            if (!statements.empty())
                statements += "\n";
            statements += stmt;
        }

        return statements;
    }

    std::string LuaTranspiler::resolveFunctionName(const AstNode* node) const
    {
        if (!node || node->functionName.empty())
            return "foo";
        return node->functionName;
    }

    std::string LuaTranspiler::resolveFunctionParameters(const AstNode* node) const
    {
        if (!node || node->variableName.empty())
            return "x";
        return node->variableName;
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
