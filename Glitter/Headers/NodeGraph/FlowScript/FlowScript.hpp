//
// Created by subha on 25-03-2026.
//

#ifndef GLITTER_FLOWSCRIPT_HPP
#define GLITTER_FLOWSCRIPT_HPP

#include <NodeGraph/NodeGraph.hpp>

// A specialized NodeGraph for authoring flow scripts and compiling them to Lua.
class FlowScript final : public NodeGraph
{
public:
    FlowScript();
    ~FlowScript() override = default;

    // Compile the current visual script into a Lua script string.
    // The result is stored internally and also returned for convenience.
    const std::string& compile();
    [[nodiscard]] const std::string& getCompiledLua() const { return compiledLua; }

private:
    std::string compiledLua;
};

#endif // GLITTER_FLOWSCRIPT_HPP

