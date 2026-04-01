#pragma once

#include <optional>
#include <string>

#include "NodeGraph/FlowScript/Decompile/ParsedStatement.hpp"

namespace Flowscript::Decompile
{
    class LuaSubsetLineParser
    {
    public:
        std::optional<ParsedStatement> parse(const std::string& rawLine) const;
    };
}

