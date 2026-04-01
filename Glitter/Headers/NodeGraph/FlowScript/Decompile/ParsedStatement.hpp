#pragma once

#include <string>

namespace Flowscript::Decompile
{
    enum class ParsedStatementKind
    {
        LocalAssignment,
        FieldAssignment,
        Print,
        Return,
        End,
    };

    struct ParsedLocalAssignment
    {
        std::string rawLine;
        std::string variableName;
        std::string expression;
    };

    struct ParsedFieldAssignment
    {
        std::string rawLine;
        std::string objectName;
        std::string fieldName;
        std::string value;
    };

    struct ParsedPrintStatement
    {
        std::string rawLine;
        std::string expression;
    };

    struct ParsedReturnStatement
    {
        std::string rawLine;
        std::string expression;
    };

    struct ParsedStatement
    {
        ParsedStatementKind kind = ParsedStatementKind::End;
        ParsedLocalAssignment local;
        ParsedFieldAssignment field;
        ParsedPrintStatement print;
        ParsedReturnStatement ret;
    };
}

