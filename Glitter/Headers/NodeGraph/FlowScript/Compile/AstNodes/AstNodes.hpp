#ifndef GLITTER_AST_NODES_HPP
#define GLITTER_AST_NODES_HPP

#include <memory>
#include <string>
#include <vector>

namespace Flowscript::Compile
{
    class AstNode
    {
    public:
        virtual ~AstNode() = default;
    };

    class StatementAstNode : public AstNode
    {
    public:
        std::vector<std::unique_ptr<StatementAstNode>> outputExecutionFlows;
        ~StatementAstNode() override = default;
    };

    class ExpressionAstNode : public AstNode
    {
    public:
        ~ExpressionAstNode() override = default;
    };

    enum class VariableValueType
    {
        Boolean,
        Number,
        String,
        Table, // Table is the declaration type ( whose value can be Array, Record, etc.). This might not be needed then.
        Array,
        Record
    };

    class FunctionStatementAstNode final : public StatementAstNode
    {
    public:
        std::string functionName;
        std::vector<std::string> parameters;
    };

    class PrintStatementAstNode final : public StatementAstNode
    {
    public:
        std::unique_ptr<ExpressionAstNode> expression;
    };

    class ReturnStatementAstNode final : public StatementAstNode
    {
    public:
        std::unique_ptr<ExpressionAstNode> expression;
    };

    class VariableDeclarationStatementAstNode final : public StatementAstNode
    {
    public:
        std::string name;
        VariableValueType valueType = VariableValueType::Number;
        std::string value;
    };

    class IntegerLiteralExpressionAstNode final : public ExpressionAstNode
    {
    public:
        std::string value;
    };

    class BooleanLiteralExpressionAstNode final : public ExpressionAstNode
    {
    public:
        std::string value;
    };

    class GetVariableExpressionAstNode final : public ExpressionAstNode
    {
    public:
        std::string name;
    };

    class BinaryExpressionAstNode : public ExpressionAstNode
    {
    public:
        std::unique_ptr<ExpressionAstNode> lhs;
        std::unique_ptr<ExpressionAstNode> rhs;
    };

    class AddExpressionAstNode final : public BinaryExpressionAstNode {};
    class SubtractExpressionAstNode final : public BinaryExpressionAstNode {};
    class MultiplyExpressionAstNode final : public BinaryExpressionAstNode {};
    class DivideExpressionAstNode final : public BinaryExpressionAstNode {};
    class ModuloExpressionAstNode final : public BinaryExpressionAstNode {};
    class LessThanExpressionAstNode final : public BinaryExpressionAstNode {};
    class EqualsToExpressionAstNode final : public BinaryExpressionAstNode {};
    class GreaterThanExpressionAstNode final : public BinaryExpressionAstNode {};
    class NotEqualsToExpressionAstNode final : public BinaryExpressionAstNode {};
}

#endif // GLITTER_AST_NODES_HPP

