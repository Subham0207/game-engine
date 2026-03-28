//
// Created by subha on 28-03-2026.
//

#ifndef GLITTER_STATEMENT_HPP
#define GLITTER_STATEMENT_HPP
#include <memory>
#include <string>
#include <vector>

#include "Expression.hpp"

namespace Flowscript::Compile
{
    struct Stmt { virtual ~Stmt() = default; };

    struct LocalAssignStmt : Stmt
    {
        std::string variableName;
        std::unique_ptr<Expr> value;
    };

    struct LocalTableStmt : Stmt
    {
        std::string variableName;
    };

    struct PrintStmt : Stmt
    {
        std::unique_ptr<Expr> value;
    };

    struct ReturnStmt : Stmt
    {
        std::unique_ptr<Expr> value;
    };

    struct FunctionStmt : Stmt
    {
        std::string name;
        std::vector<std::unique_ptr<Stmt>> body;
    };
}
#endif //GLITTER_STATEMENT_HPP