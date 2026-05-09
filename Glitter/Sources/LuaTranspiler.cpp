//
// Created by subha on 03-05-2026.
//

#include "../Headers/NodeGraph/FlowScript/Compile/LuaTranspiler.hpp"
#include <stdexcept>

namespace Flowscript::Compile
{
    std::string LuaTranspiler::Transpile(const std::vector<std::unique_ptr<StatementAstNode>>& ast)
    {
        std::string luaCode;
        for (const auto& node: ast)
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

        if (const auto* functionNode = dynamic_cast<const FunctionStatementAstNode*>(node))
            return transpileFunctionNode(functionNode);
        if (const auto* printNode = dynamic_cast<const PrintStatementAstNode*>(node))
            return transpilePrintNode(printNode);
        if (const auto* returnNode = dynamic_cast<const ReturnStatementAstNode*>(node))
            return transpileReturnNode(returnNode);
        if (const auto* variableDeclNode = dynamic_cast<const VariableDeclarationStatementAstNode*>(node))
            return transpileVariableDeclarationNode(variableDeclNode);

        return "";
    }

    std::string LuaTranspiler::transpileFunctionNode(const FunctionStatementAstNode* node)
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

    std::string LuaTranspiler::transpilePrintNode(const PrintStatementAstNode* node)
    {
        std::string expr;
        if (node && node->expression)
            expr = recurseExpression(node->expression.get());

        return transpileStatementWithTrailingExecution(node, "print(" + expr + ")");
    }

    std::string LuaTranspiler::transpileReturnNode(const ReturnStatementAstNode* node)
    {
        std::string currentStatement = "return";
        if (node && node->expression)
        {
            const std::string expr = recurseExpression(node->expression.get());
            currentStatement = expr.empty() ? "return" : ("return " + expr);
        }

        // Return terminates the execution chain in generated Lua.
        return currentStatement;
    }

    std::string LuaTranspiler::transpileVariableDeclarationNode(const VariableDeclarationStatementAstNode* node)
    {
        if (!node || node->name.empty())
            throw std::runtime_error("Variable declaration requires a name");

        return transpileStatementWithTrailingExecution(
            node,
            "local " + node->name + " = " + resolveVariableDeclarationValue(node)
        );
    }

    std::string LuaTranspiler::transpileStatementWithTrailingExecution(
        const StatementAstNode* node,
        const std::string& currentStatement
    )
    {
        const std::string trailingStatements = transpileExecutionFlowChildren(node);
        if (trailingStatements.empty())
            return currentStatement;

        return currentStatement + "\n" + trailingStatements;
    }

    std::string LuaTranspiler::transpileExecutionFlowChildren(const StatementAstNode* node)
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

    std::string LuaTranspiler::resolveFunctionName(const FunctionStatementAstNode* node)
    {
        if (!node || node->functionName.empty())
            throw std::runtime_error("Function statement requires a function name");
        return node->functionName;
    }

    std::string LuaTranspiler::resolveFunctionParameters(const FunctionStatementAstNode* node)
    {
        if (!node || node->parameters.empty())
            return "";

        std::string parameters;
        for (const std::string& name: node->parameters)
        {
            if (name.empty())
                continue;
            if (!parameters.empty())
                parameters += ",";
            parameters += name;
        }

        return parameters;
    }

    std::string LuaTranspiler::resolveVariableDeclarationValue(const VariableDeclarationStatementAstNode* node) const
    {
        if (!node)
            return "nil";

        if (!node->value.empty())
            return node->value;

        switch (node->valueType)
        {
            case VariableValueType::Boolean:
                return "false";
            case VariableValueType::Number:
                return "0";
            case VariableValueType::String:
                return "\"\"";
            case VariableValueType::Table:
            case VariableValueType::Array:
            case VariableValueType::Record:
                return "{}";
        }

        return "nil";
    }

    std::string LuaTranspiler::transpileBinaryExpression(const BinaryExpressionAstNode* node, const std::string& op)
    {
        if (!node)
            return "";

        const std::string lhs = recurseExpression(node->lhs.get());
        const std::string rhs = recurseExpression(node->rhs.get());
        return "(" + lhs + " " + op + " " + rhs + ")";
    }

    std::string LuaTranspiler::recurseExpression(const ExpressionAstNode* node)
    {
        if (!node)
            return "";

        if (const auto* integerLiteral = dynamic_cast<const IntegerLiteralExpressionAstNode*>(node))
            return integerLiteral->value.empty() ? "0" : integerLiteral->value;
        if (const auto* booleanLiteral = dynamic_cast<const BooleanLiteralExpressionAstNode*>(node))
            return booleanLiteral->value.empty() ? "false" : booleanLiteral->value;
        if (const auto* getVariable = dynamic_cast<const GetVariableExpressionAstNode*>(node))
        {
            if (getVariable->name.empty())
                throw std::runtime_error("GetVariable expression requires a variable name");
            return getVariable->name;
        }
        if (const auto* addExpr = dynamic_cast<const AddExpressionAstNode*>(node))
            return transpileBinaryExpression(addExpr, "+");
        if (const auto* subtractExpr = dynamic_cast<const SubtractExpressionAstNode*>(node))
            return transpileBinaryExpression(subtractExpr, "-");
        if (const auto* multiplyExpr = dynamic_cast<const MultiplyExpressionAstNode*>(node))
            return transpileBinaryExpression(multiplyExpr, "*");
        if (const auto* divideExpr = dynamic_cast<const DivideExpressionAstNode*>(node))
            return transpileBinaryExpression(divideExpr, "/");
        if (const auto* moduloExpr = dynamic_cast<const ModuloExpressionAstNode*>(node))
            return transpileBinaryExpression(moduloExpr, "%");
        if (const auto* lessThanExpr = dynamic_cast<const LessThanExpressionAstNode*>(node))
            return transpileBinaryExpression(lessThanExpr, "<");
        if (const auto* equalsExpr = dynamic_cast<const EqualsToExpressionAstNode*>(node))
            return transpileBinaryExpression(equalsExpr, "==");
        if (const auto* greaterExpr = dynamic_cast<const GreaterThanExpressionAstNode*>(node))
            return transpileBinaryExpression(greaterExpr, ">");
        if (const auto* notEqualsExpr = dynamic_cast<const NotEqualsToExpressionAstNode*>(node))
            return transpileBinaryExpression(notEqualsExpr, "~=");

        return "";
    }
}
