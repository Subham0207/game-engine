//
// Created by subha on 28-03-2026.
//

#include "../Headers/NodeGraph/FlowScript/LuaEmitter.hpp"

#include <algorithm>

using namespace Flowscript::Compile;

std::string LuaEmitter::Emit(const std::vector<std::unique_ptr<Stmt>>& statements)
{
    std::ostringstream out;
    for (const auto& stmt : statements)
    {
        if (stmt)
            EmitStmt(out, *stmt, 0);
    }
    return out.str();
}

std::string LuaEmitter::EmitExpr(const Expr& expr)
{
    if (const auto* number = dynamic_cast<const NumberExpr*>(&expr))
        return number->value;

    if (const auto* boolean = dynamic_cast<const BoolExpr*>(&expr))
        return boolean->value ? "true" : "false";

    if (const auto* var = dynamic_cast<const VariableExpr*>(&expr))
        return var->name;

    if (const auto* binary = dynamic_cast<const BinaryExpr*>(&expr))
    {
        const std::string lhs = binary->lhs ? EmitExpr(*binary->lhs) : "0";
        const std::string rhs = binary->rhs ? EmitExpr(*binary->rhs) : "0";
        return "(" + lhs + " " + binary->op + " " + rhs + ")";
    }

    return "0";
}

void LuaEmitter::EmitStmt(std::ostringstream& out, const Stmt& stmt, const int indent)
{
    const std::string pad = Indent(indent);

    if (const auto* localAssign = dynamic_cast<const LocalAssignStmt*>(&stmt))
    {
        const std::string value = localAssign->value ? EmitExpr(*localAssign->value) : "0";
        if (localAssign->variableName.find('.') != std::string::npos)
            out << pad << localAssign->variableName << " = " << value << "\n";
        else
            out << pad << "local " << localAssign->variableName << " = " << value << "\n";
        return;
    }

    if (const auto* localTable = dynamic_cast<const LocalTableStmt*>(&stmt))
    {
        out << pad << "local " << localTable->variableName << " = {}\n";
        return;
    }

    if (const auto* printStmt = dynamic_cast<const PrintStmt*>(&stmt))
    {
        const std::string value = printStmt->value ? EmitExpr(*printStmt->value) : "0";
        out << pad << "print(\"[LUA]\", " << value << ")\n";
        return;
    }

    if (const auto* returnStmt = dynamic_cast<const ReturnStmt*>(&stmt))
    {
        const std::string value = returnStmt->value ? EmitExpr(*returnStmt->value) : "0";
        out << pad << "return " << value << "\n";
        return;
    }

    if (const auto* functionStmt = dynamic_cast<const FunctionStmt*>(&stmt))
    {
        out << pad << "local " << functionStmt->name << " = function()\n";
        for (const auto& bodyStmt : functionStmt->body)
        {
            if (bodyStmt)
                EmitStmt(out, *bodyStmt, indent + 1);
        }
        out << pad << "end\n";
        return;
    }
}

std::string LuaEmitter::Indent(const int depth)
{
    return std::string(static_cast<size_t>(std::max(0, depth)) * 4u, ' ');
}
