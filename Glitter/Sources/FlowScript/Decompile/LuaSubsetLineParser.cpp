#include "NodeGraph/FlowScript/Decompile/LuaSubsetLineParser.hpp"
#include "NodeGraph/FlowScript/Decompile/Helpers.hpp"

#include <stdexcept>

namespace
{
    std::string stripInlineLuaComment(const std::string& value)
    {
        bool inSingleQuote = false;
        bool inDoubleQuote = false;

        for (size_t i = 0; i + 1 < value.size(); ++i)
        {
            const char c = value[i];
            if (c == '\\')
            {
                ++i;
                continue;
            }
            if (!inDoubleQuote && c == '\'')
            {
                inSingleQuote = !inSingleQuote;
                continue;
            }
            if (!inSingleQuote && c == '"')
            {
                inDoubleQuote = !inDoubleQuote;
                continue;
            }
            if (!inSingleQuote && !inDoubleQuote && c == '-' && value[i + 1] == '-')
                return value.substr(0, i);
        }

        return value;
    }

    std::string normalizeLine(const std::string& rawLine)
    {
        std::string line = rawLine;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        return Flowscript::Decompile::Helpers::trimCopy(stripInlineLuaComment(line));
    }

    Flowscript::Decompile::ParsedStatement parseLocal(const std::string& line)
    {
        std::string rest = Flowscript::Decompile::Helpers::trimCopy(line.substr(6));
        const size_t eqPos = rest.find('=');
        if (eqPos == std::string::npos)
            throw std::runtime_error("Invalid assignment: " + line);

        Flowscript::Decompile::ParsedStatement statement;
        statement.kind = Flowscript::Decompile::ParsedStatementKind::LocalAssignment;
        statement.local.rawLine = line;
        statement.local.variableName = Flowscript::Decompile::Helpers::trimCopy(rest.substr(0, eqPos));
        statement.local.expression = Flowscript::Decompile::Helpers::trimCopy(rest.substr(eqPos + 1));
        return statement;
    }

    std::optional<Flowscript::Decompile::ParsedStatement> tryParseFieldAssignment(const std::string& line)
    {
        const size_t eqPos = line.find('=');
        if (eqPos == std::string::npos)
            return std::nullopt;

        const std::string lhs = Flowscript::Decompile::Helpers::trimCopy(line.substr(0, eqPos));
        const std::string rhs = Flowscript::Decompile::Helpers::trimCopy(line.substr(eqPos + 1));

        const size_t dotPos = lhs.find('.');
        if (dotPos == std::string::npos || dotPos == 0 || dotPos + 1 >= lhs.size())
            return std::nullopt;

        Flowscript::Decompile::ParsedStatement statement;
        statement.kind = Flowscript::Decompile::ParsedStatementKind::FieldAssignment;
        statement.field.rawLine = line;
        statement.field.objectName = Flowscript::Decompile::Helpers::trimCopy(lhs.substr(0, dotPos));
        statement.field.fieldName = Flowscript::Decompile::Helpers::trimCopy(lhs.substr(dotPos + 1));
        statement.field.value = rhs;
        return statement;
    }

    Flowscript::Decompile::ParsedStatement parsePrint(const std::string& line)
    {
        const size_t commaPos = line.find(',');
        const size_t closePos = line.rfind(')');
        if (commaPos == std::string::npos || closePos == std::string::npos || closePos <= commaPos)
            throw std::runtime_error("Unsupported print format: " + line);

        Flowscript::Decompile::ParsedStatement statement;
        statement.kind = Flowscript::Decompile::ParsedStatementKind::Print;
        statement.print.rawLine = line;
        statement.print.expression = Flowscript::Decompile::Helpers::trimCopy(line.substr(commaPos + 1, closePos - commaPos - 1));
        return statement;
    }

    Flowscript::Decompile::ParsedStatement parseReturn(const std::string& line)
    {
        Flowscript::Decompile::ParsedStatement statement;
        statement.kind = Flowscript::Decompile::ParsedStatementKind::Return;
        statement.ret.rawLine = line;
        statement.ret.expression = Flowscript::Decompile::Helpers::trimCopy(line.substr(7));
        return statement;
    }
}

namespace Flowscript::Decompile
{
    std::optional<ParsedStatement> LuaSubsetLineParser::parse(const std::string& rawLine) const
    {
        if (rawLine.find("-- unsupported node: GenericType(") != std::string::npos)
            return std::nullopt;

        const std::string line = normalizeLine(rawLine);
        if (line.empty() || Helpers::startsWith(line, "--"))
            return std::nullopt;

        if (Helpers::startsWith(line, "local "))
            return parseLocal(line);

        if (auto field = tryParseFieldAssignment(line))
            return field;

        if (Helpers::startsWith(line, "print("))
            return parsePrint(line);

        if (Helpers::startsWith(line, "return "))
            return parseReturn(line);

        if (line == "end")
        {
            ParsedStatement statement;
            statement.kind = ParsedStatementKind::End;
            return statement;
        }

        throw std::runtime_error("Unsupported Lua statement: " + line);
    }
}


