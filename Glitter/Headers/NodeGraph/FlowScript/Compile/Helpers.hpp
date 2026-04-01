//
// Created by subha on 28-03-2026.
//

#ifndef GLITTER_HELPERS_HPP
#define GLITTER_HELPERS_HPP
#pragma once
#include <algorithm>
#include <stdexcept>
#include <string>
namespace Flowscript::Compile
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

    inline std::string parseNumberOrDefault(const char* value)
    {
        if (!value)
            return "0";
        std::string trimmed = trimCopy(value);
        if (trimmed.empty())
            return "0";
        char* end = nullptr;
        std::strtod(trimmed.c_str(), &end);
        if (!end || *end != '\0')
            return "0";
        return trimmed;
    }

    inline bool parseBooleanOrDefault(const char* value)
    {
        if (!value)
            return false;
        std::string trimmed = trimCopy(value);
        if (trimmed.empty())
            return false;
        std::transform(trimmed.begin(), trimmed.end(), trimmed.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (trimmed == "1" || trimmed == "true" || trimmed == "yes" || trimmed == "on")
            return true;
        return false;
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

    inline bool isGenericTypeNodeName(const std::string& value)
    {
        return startsWith(value, "GenericType(") && value.size() > std::strlen("GenericType(") && value.back() == ')';
    }

    inline std::string getGenericObjectName(const std::string& nodeName)
    {
        if (!isGenericTypeNodeName(nodeName))
            return {};
        const size_t prefixLen = std::strlen("GenericType(");
        return nodeName.substr(prefixLen, nodeName.size() - prefixLen - 1);
    }

    inline std::string stripInlineLuaComment(const std::string& value)
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
}
#endif //GLITTER_HELPERS_HPP