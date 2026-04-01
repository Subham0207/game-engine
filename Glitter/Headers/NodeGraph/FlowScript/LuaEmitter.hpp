//
// Created by subha on 28-03-2026.
//

#ifndef GLITTER_LUAEMITTER_HPP
#define GLITTER_LUAEMITTER_HPP
#pragma once
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "Compile/IntermediateRepresentation/Expression.hpp"
#include "Compile/IntermediateRepresentation/Statement.hpp"

class LuaEmitter
{
public:
    std::string Emit(const std::vector<std::unique_ptr<Flowscript::Compile::Stmt>>& statements);
    std::string EmitExpr(const Flowscript::Compile::Expr& expr);
    void EmitStmt(std::ostringstream& out, const Flowscript::Compile::Stmt& stmt, int indent);

private:
    static std::string Indent(int depth);
};


#endif //GLITTER_LUAEMITTER_HPP

