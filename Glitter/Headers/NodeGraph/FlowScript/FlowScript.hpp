//
// Created by subha on 25-03-2026.
//

#ifndef GLITTER_FLOWSCRIPT_HPP
#define GLITTER_FLOWSCRIPT_HPP
#pragma once
#include <NodeGraph/NodeGraph.hpp>
#include <vector>

// A specialized NodeGraph for authoring flow scripts and compiling them to Lua.
class FlowScript : public NodeGraph
{
public:
    FlowScript();
    ~FlowScript() override = default;

    // Compile the current visual script into a Lua script string.
    // The result is stored internally and also returned for convenience.
    virtual const std::string& compile();
    [[nodiscard]] const std::string& getCompiledLua() const { return compiledLua; }
    void setCompiledLua(const std::string& codeString) { compiledLua = codeString; }

private:
    void appendLuaLog(const std::string& line);

    std::string compiledLua;

    std::vector<std::string> luaConsoleLines;
    bool luaConsoleScrollToBottom = false;
    static constexpr size_t kLuaConsoleMaxLines = 200;
};

#endif // GLITTER_FLOWSCRIPT_HPP

