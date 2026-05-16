//
// Created by subha on 25-03-2026.
//

#ifndef GLITTER_FLOWSCRIPT_HPP
#define GLITTER_FLOWSCRIPT_HPP
#pragma once
#include <NodeGraph/NodeGraph.hpp>
#include <filesystem>
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
    [[nodiscard]] const std::vector<std::string>& getCompileDiagnostics() const { return compileDiagnostics; }
    void setCompiledLua(const std::string& codeString) { compiledLua = codeString; }
    void clearScript();
    void deCompile(const std::string& luaCode);
    bool saveVisualScriptToFile(const std::filesystem::path& filePath) const;
    bool loadVisualScriptFromFile(const std::filesystem::path& filePath);

private:
    void appendLuaLog(const std::string& line);
    void syncNodeIdAllocatorsAfterLoad();

    std::string compiledLua;
    std::vector<std::string> compileDiagnostics;

    std::vector<std::string> luaConsoleLines;
    bool luaConsoleScrollToBottom = false;
    static constexpr size_t kLuaConsoleMaxLines = 200;
};

#endif // GLITTER_FLOWSCRIPT_HPP

