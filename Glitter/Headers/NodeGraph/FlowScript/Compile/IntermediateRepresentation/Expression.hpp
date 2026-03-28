//
// Created by subha on 28-03-2026.
//

#ifndef GLITTER_EXPRESSION_HPP
#define GLITTER_EXPRESSION_HPP
#pragma once
#include <memory>
#include <string>

namespace Flowscript::Compile
{
    struct Expr { virtual ~Expr() = default; };

    struct NumberExpr : Expr
    {
        std::string value;
    };

    struct BoolExpr : Expr
    {
        bool value = false;
    };

    struct VariableExpr : Expr
    {
        std::string name;
    };

    struct BinaryExpr : Expr
    {
        std::string op;
        std::unique_ptr<Expr> lhs;
        std::unique_ptr<Expr> rhs;
    };
}
#endif //GLITTER_EXPRESSION_HPP