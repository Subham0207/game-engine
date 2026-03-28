#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>

namespace Flowscript::Decompile::Helpers
{
    inline std::string trimCopy(const std::string& value)
    {
        size_t start = 0;
        while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])))
            ++start;

        size_t end = value.size();
        while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])))
            --end;

        return value.substr(start, end - start);
    }

    inline bool startsWith(const std::string& value, const char* prefix)
    {
        if (!prefix)
            return false;

        const size_t prefixLen = std::strlen(prefix);
        if (value.size() < prefixLen)
            return false;

        return std::equal(prefix, prefix + prefixLen, value.begin());
    }

    inline bool isNumberLiteral(const std::string& value)
    {
        if (value.empty())
            return false;

        char* end = nullptr;
        std::strtod(value.c_str(), &end);
        return end != value.c_str() && *end == '\0';
    }

    inline bool isBooleanLiteral(const std::string& value)
    {
        return value == "true" || value == "false";
    }

    inline std::pair<std::string, std::string> splitOnce(const std::string& value, const std::string& token)
    {
        const size_t pos = value.find(token);
        if (pos == std::string::npos)
            return { value, "" };

        const size_t nextPos = value.find(token, pos + token.size());
        if (nextPos != std::string::npos)
            throw std::runtime_error("Unsupported expression: multiple operators");

        return { value.substr(0, pos), value.substr(pos + token.size()) };
    }
}

