//
// Created by subha on 25-03-2026.
//

#include "../Headers/NodeGraph/FlowScript/FlowScript.hpp"
#include <EngineState.hpp>
#include <imgui.h>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <stdexcept>
#include <cstring>

#include "NodeGraph/Views/NodeGraphNodesView.hpp"
#include "NodeGraph/FlowScript/Compile/Compiler.hpp"
#include "NodeGraph/FlowScript/LuaEmitter.hpp"

namespace
{
    std::string trimCopy(const std::string& value)
    {
        size_t start = 0;
        while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])))
            ++start;
        size_t end = value.size();
        while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])))
            --end;
        return value.substr(start, end - start);
    }

    std::string parseNumberOrDefault(const char* value)
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

    std::string parseBooleanOrDefault(const char* value)
    {
        if (!value)
            return "false";
        std::string trimmed = trimCopy(value);
        if (trimmed.empty())
            return "false";
        std::transform(trimmed.begin(), trimmed.end(), trimmed.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (trimmed == "1" || trimmed == "true" || trimmed == "yes" || trimmed == "on")
            return "true";
        return "false";
    }

    bool startsWith(const std::string& value, const char* prefix)
    {
        if (!prefix)
            return false;
        const size_t prefixLen = std::strlen(prefix);
        if (value.size() < prefixLen)
            return false;
        return std::equal(prefix, prefix + prefixLen, value.begin());
    }

    bool isNumberLiteral(const std::string& value)
    {
        if (value.empty())
            return false;
        char* end = nullptr;
        std::strtod(value.c_str(), &end);
        return end != value.c_str() && *end == '\0';
    }

    bool isBooleanLiteral(const std::string& value)
    {
        return value == "true" || value == "false";
    }

    std::pair<std::string, std::string> splitOnce(const std::string& value, const std::string& token)
    {
        const size_t pos = value.find(token);
        if (pos == std::string::npos)
            return { value, "" };
        const size_t nextPos = value.find(token, pos + token.size());
        if (nextPos != std::string::npos)
            throw std::runtime_error("Unsupported expression: multiple operators");
        return { value.substr(0, pos), value.substr(pos + token.size()) };
    }

    bool isGenericTypeNodeName(const std::string& value)
    {
        return startsWith(value, "GenericType(") && value.size() > std::strlen("GenericType(") && value.back() == ')';
    }

    std::string getGenericObjectName(const std::string& nodeName)
    {
        if (!isGenericTypeNodeName(nodeName))
            return {};
        const size_t prefixLen = std::strlen("GenericType(");
        return nodeName.substr(prefixLen, nodeName.size() - prefixLen - 1);
    }

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
}

FlowScript::FlowScript()
{
    setScreenSpaceUi(
        [](NodeGraphRenderContext& ctx, void* user) {
            auto* self = static_cast<FlowScript*>(user);

            ImGui::SetCursorPos(ImVec2(8.0f, 8.0f));
            ImGui::BeginGroup();

            if (ImGui::SmallButton("Compile"))
                self->compile();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("Compiles the graph to Lua and stores the result");

            const bool hasCompiled = !self->compiledLua.empty();
            if (!hasCompiled)
                ImGui::BeginDisabled();

            if (ImGui::SmallButton("Copy Lua"))
            {
                if (!self->compiledLua.empty())
                    ImGui::SetClipboardText(self->compiledLua.c_str());
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("Copies the compiled Lua to the clipboard");

            if (ImGui::SmallButton("Execute"))
            {
                if (!self->compiledLua.empty())
                {
                    getLuaEngine().setPrintHandler([self](const std::string& line) {
                        self->appendLuaLog(line);
                    });
                    try
                    {
                        getLuaEngine().runChunk(self->compiledLua, "FlowScript");
                    }
                    catch (const std::exception& ex)
                    {
                        self->appendLuaLog(std::string("[LUA] ") + ex.what());
                    }
                }
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("Executes the compiled Lua in the Lua runtime");

            if (!hasCompiled)
                ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::SmallButton("Clear Log"))
            {
                self->luaConsoleLines.clear();
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("Clears the Lua console output");

            ImGui::EndGroup();

            ImGui::SetCursorPos(ImVec2(8.0f, 72.0f));
            ImGui::BeginChild("LuaConsole", ImVec2(360.0f, 140.0f), true);
            for (const auto& line : self->luaConsoleLines)
                ImGui::TextUnformatted(line.c_str());
            if (self->luaConsoleScrollToBottom)
            {
                ImGui::SetScrollHereY(1.0f);
                self->luaConsoleScrollToBottom = false;
            }
            ImGui::EndChild();
        },
        this);
}

void FlowScript::clearScript()
{
    clearNodes();
    clearNodeGraphLinks();
    clearComments();
}

const std::string& FlowScript::compile()
{
    Flowscript::Compile::Compiler compiler;
    const Flowscript::Compile::CompileResult result = compiler.Compile(nodes, nodeGraphLinks);

    compileDiagnostics = result.diagnostics;

    LuaEmitter emitter;
    std::string emittedLua = emitter.Emit(result.statements);

    if (!emittedLua.empty())
        compiledLua = "-- Generated by FlowScript::compile()\n" + emittedLua;
    else
        compiledLua = "-- Generated by FlowScript::compile()\n";

    for (const auto& message : compileDiagnostics)
        appendLuaLog(std::string("[COMPILE] ") + message);

    return compiledLua;
}

void FlowScript::appendLuaLog(const std::string& line)
{
    if (luaConsoleLines.size() >= kLuaConsoleMaxLines)
        luaConsoleLines.erase(luaConsoleLines.begin());
    luaConsoleLines.push_back(line);
    luaConsoleScrollToBottom = true;
}

void FlowScript::deCompile(const std::string& luaCode)
{
    auto* nodeView = views.findView<NodeGraphNodesView>();
    if (!nodeView)
        throw std::runtime_error("NodeGraphNodesView not available");

    struct NodeOutputRef
    {
        NodeGraphNode* node = nullptr;
        int outputAttr = -1;
    };

    struct PendingGenericObject
    {
        std::vector<NodeGraphComponents::Node::GenericMemberSpec> members;
        std::unordered_map<std::string, size_t> fieldToMemberIndex;
    };

    std::unordered_map<std::string, NodeOutputRef> vars;
    std::vector<NodeGraphNode*> topLevelExec;

    int layoutX = 0;
    int layoutY = 0;
    const float stepX = 220.0f;
    const float stepY = 140.0f;
    auto nextPos = [&]() {
        ImVec2 pos(120.0f + stepX * static_cast<float>(layoutX), 120.0f + stepY * static_cast<float>(layoutY));
        layoutX++;
        if (layoutX >= 4)
        {
            layoutX = 0;
            layoutY++;
        }
        return pos;
    };

    auto addNode = [&](NodeTypes type) -> NodeGraphNode* {
        const size_t before = nodes.size();
        nodeView->addNode(nodes, type, nextPos());
        if (nodes.size() <= before)
            throw std::runtime_error("Failed to create node");
        return nodes.back().get();
    };

    auto addGenericTypeNode = [&](const std::string& objectName,
                                  const std::vector<NodeGraphComponents::Node::GenericMemberSpec>& members) -> NodeGraphNode* {
        const size_t before = nodes.size();
        nodeView->addGenericTypeNode(nodes, objectName, members, nextPos());
        if (nodes.size() <= before)
            throw std::runtime_error("Failed to create generic object node");
        return nodes.back().get();
    };

    auto setFieldValue = [&](NodeGraphNode* node, const std::string& value) {
        if (!node || node->outputs().empty())
            throw std::runtime_error("Node has no outputs to set");
        char* buff = node->outputs()[0].getValueBuff();
        std::snprintf(buff, NodeGraphComponents::Node::Attribute::getValueSize(), "%s", value.c_str());
    };

    auto getOutputAttr = [&](NodeGraphNode* node) -> int {
        if (!node || node->outputs().empty())
            throw std::runtime_error("Node has no output attribute");
        return node->outputs()[0].getId();
    };

    auto resolveValue = [&](const std::string& raw) -> NodeOutputRef {
        const std::string value = trimCopy(raw);
        {
            const auto it = vars.find(value);
            if (it != vars.end() && it->second.node && it->second.outputAttr >= 0)
                return it->second;
        }
        if (startsWith(value, "node_"))
        {
            const auto it = vars.find(value);
            if (it == vars.end() || !it->second.node || it->second.outputAttr < 0)
                throw std::runtime_error("Unknown variable reference: " + value);
            return it->second;
        }
        if (isBooleanLiteral(value))
        {
            auto* node = addNode(NodeTypes::Boolean);
            setFieldValue(node, value);
            return { node, getOutputAttr(node) };
        }
        if (isNumberLiteral(value))
        {
            auto* node = addNode(NodeTypes::Integer);
            setFieldValue(node, value);
            return { node, getOutputAttr(node) };
        }
        throw std::runtime_error("Unsupported literal: " + value);
    };

    auto linkDataInputs = [&](NodeGraphNode* node, const std::vector<std::string>& inputs) {
        if (!node)
            return;
        auto& nodeInputs = node->inputs();
        if (nodeInputs.size() < inputs.size())
            throw std::runtime_error("Node input arity mismatch");
        for (size_t i = 0; i < inputs.size(); ++i)
        {
            const auto ref = resolveValue(inputs[i]);
            nodeView->addLink(nodeGraphLinks, ref.outputAttr, nodeInputs[i].getId());
        }
    };

    auto linkExecChain = [&](const std::vector<NodeGraphNode*>& chain) {
        NodeGraphNode* prev = nullptr;
        for (auto* node : chain)
        {
            if (!node)
                continue;
            NodeAttribute* in = node->getExecInput();
            NodeAttribute* out = node->getExecOutput();
            if (prev && in && prev->getExecOutput())
                nodeView->addLink(nodeGraphLinks, prev->getExecOutput()->getId(), in->getId());
            if (out)
                prev = node;
            else
                prev = nullptr;
        }
    };

    struct FunctionScope
    {
        NodeGraphNode* functionNode = nullptr;
        std::vector<NodeGraphNode*> execChain;
    };

    std::vector<FunctionScope> functionStack;
    std::unordered_map<std::string, PendingGenericObject> pendingGenericObjects;
    std::vector<std::string> pendingGenericOrder;

    auto registerGenericField = [&](const std::string& objectName,
                                    const std::string& fieldName,
                                    const std::string& literalValue) {
        auto objectIt = pendingGenericObjects.find(objectName);
        if (objectIt == pendingGenericObjects.end())
            return false;

        const bool isBool = isBooleanLiteral(literalValue);
        const bool isNumber = isNumberLiteral(literalValue);
        if (!isBool && !isNumber)
            return true; // Ignore unsupported literals for GenericType v1.

        auto& pending = objectIt->second;
        auto fieldIt = pending.fieldToMemberIndex.find(fieldName);
        if (fieldIt == pending.fieldToMemberIndex.end())
        {
            NodeGraphComponents::Node::GenericMemberSpec spec;
            spec.name = fieldName;
            spec.literalValue = literalValue;
            spec.isBoolean = isBool;

            pending.fieldToMemberIndex[fieldName] = pending.members.size();
            pending.members.push_back(std::move(spec));
        }
        else
        {
            auto& spec = pending.members[fieldIt->second];
            spec.literalValue = literalValue;
            spec.isBoolean = isBool;
        }

        return true;
    };

    auto flushPendingGenericObjects = [&]() {
        for (const auto& objectName : pendingGenericOrder)
        {
            const auto objectIt = pendingGenericObjects.find(objectName);
            if (objectIt == pendingGenericObjects.end())
                continue;

            const auto& members = objectIt->second.members;
            if (members.empty())
                continue;

            auto* genericNode = addGenericTypeNode(objectName, members);
            auto& outputs = genericNode->outputs();
            const size_t count = std::min(outputs.size(), members.size());
            for (size_t i = 0; i < count; ++i)
                vars[objectName + "." + members[i].name] = { genericNode, outputs[i].getId() };
        }

        pendingGenericObjects.clear();
        pendingGenericOrder.clear();
    };

    std::istringstream stream(luaCode);
    std::string rawLine;

    while (std::getline(stream, rawLine))
    {
        if (!rawLine.empty() && rawLine.back() == '\r')
            rawLine.pop_back();
        if (rawLine.find("-- unsupported node: GenericType(") != std::string::npos)
            continue;
        std::string line = trimCopy(stripInlineLuaComment(rawLine));
        if (line.empty())
            continue;
        if (startsWith(line, "--"))
            continue;

        const bool inFunction = !functionStack.empty();

        if (startsWith(line, "local "))
        {
            std::string rest = trimCopy(line.substr(6));
            const size_t eqPos = rest.find("=");
            if (eqPos == std::string::npos)
                throw std::runtime_error("Invalid assignment: " + line);
            std::string varName = trimCopy(rest.substr(0, eqPos));
            std::string expr = trimCopy(rest.substr(eqPos + 1));

            if (expr == "{}")
            {
                if (pendingGenericObjects.find(varName) == pendingGenericObjects.end())
                    pendingGenericOrder.push_back(varName);
                pendingGenericObjects[varName];
                continue;
            }

            flushPendingGenericObjects();

            if (expr == "function()")
            {
                auto* funcNode = addNode(NodeTypes::Function);
                vars[varName] = { funcNode, -1 };
                functionStack.push_back({ funcNode, {} });
                continue;
            }

            if (expr.size() >= 2 && expr.front() == '(' && expr.back() == ')')
            {
                std::string inner = trimCopy(expr.substr(1, expr.size() - 2));
                if (inner.find(" + ") != std::string::npos)
                {
                    auto parts = splitOnce(inner, " + ");
                    auto* node = addNode(NodeTypes::Add);
                    vars[varName] = { node, getOutputAttr(node) };
                    linkDataInputs(node, { parts.first, parts.second });
                    if (inFunction)
                        functionStack.back().execChain.push_back(node);
                    else
                        topLevelExec.push_back(node);
                    continue;
                }
                if (inner.find(" - ") != std::string::npos)
                {
                    auto parts = splitOnce(inner, " - ");
                    auto* node = addNode(NodeTypes::Subtract);
                    vars[varName] = { node, getOutputAttr(node) };
                    linkDataInputs(node, { parts.first, parts.second });
                    if (inFunction)
                        functionStack.back().execChain.push_back(node);
                    else
                        topLevelExec.push_back(node);
                    continue;
                }
                if (inner.find(" > ") != std::string::npos)
                {
                    auto parts = splitOnce(inner, " > ");
                    auto* node = addNode(NodeTypes::GreaterThan);
                    vars[varName] = { node, getOutputAttr(node) };
                    linkDataInputs(node, { parts.first, parts.second });
                    if (inFunction)
                        functionStack.back().execChain.push_back(node);
                    else
                        topLevelExec.push_back(node);
                    continue;
                }
                if (inner.find(" == ") != std::string::npos)
                {
                    auto parts = splitOnce(inner, " == ");
                    auto* node = addNode(NodeTypes::EqualsTo);
                    vars[varName] = { node, getOutputAttr(node) };
                    linkDataInputs(node, { parts.first, parts.second });
                    continue;
                }
                if (inner.find(" ~= ") != std::string::npos)
                {
                    auto parts = splitOnce(inner, " ~= ");
                    auto* node = addNode(NodeTypes::NotEqualsTo);
                    vars[varName] = { node, getOutputAttr(node) };
                    linkDataInputs(node, { parts.first, parts.second });
                    continue;
                }
                throw std::runtime_error("Unsupported expression: " + expr);
            }

            if (isBooleanLiteral(expr))
            {
                auto* node = addNode(NodeTypes::Boolean);
                setFieldValue(node, expr);
                vars[varName] = { node, getOutputAttr(node) };
                continue;
            }
            if (isNumberLiteral(expr))
            {
                auto* node = addNode(NodeTypes::Integer);
                setFieldValue(node, expr);
                vars[varName] = { node, getOutputAttr(node) };
                continue;
            }
            throw std::runtime_error("Unsupported assignment: " + line);
        }

        {
            const size_t eqPos = line.find('=');
            if (eqPos != std::string::npos)
            {
                const std::string lhs = trimCopy(line.substr(0, eqPos));
                const std::string rhs = trimCopy(line.substr(eqPos + 1));
                const size_t dotPos = lhs.find('.');
                if (dotPos != std::string::npos && dotPos > 0 && dotPos + 1 < lhs.size())
                {
                    const std::string objectName = trimCopy(lhs.substr(0, dotPos));
                    const std::string fieldName = trimCopy(lhs.substr(dotPos + 1));
                    if (registerGenericField(objectName, fieldName, rhs))
                        continue;
                }
            }
        }

        flushPendingGenericObjects();

        if (startsWith(line, "print("))
        {
            const size_t commaPos = line.find(",");
            const size_t closePos = line.rfind(")");
            if (commaPos == std::string::npos || closePos == std::string::npos || closePos <= commaPos)
                throw std::runtime_error("Unsupported print format: " + line);
            std::string expr = trimCopy(line.substr(commaPos + 1, closePos - commaPos - 1));
            auto* node = addNode(NodeTypes::Print);
            linkDataInputs(node, { expr });
            if (inFunction)
                functionStack.back().execChain.push_back(node);
            else
                topLevelExec.push_back(node);
            continue;
        }

        if (startsWith(line, "return "))
        {
            std::string expr = trimCopy(line.substr(7));
            if (!expr.empty() && expr.front() == '{')
                throw std::runtime_error("Return of table not supported");
            auto* node = addNode(NodeTypes::Return);
            linkDataInputs(node, { expr });
            if (inFunction)
                functionStack.back().execChain.push_back(node);
            else
                topLevelExec.push_back(node);
            continue;
        }

        if (line == "end")
        {
            if (functionStack.empty())
                throw std::runtime_error("Unexpected 'end' without function");
            auto scope = functionStack.back();
            functionStack.pop_back();
            if (!scope.execChain.empty())
            {
                auto* first = scope.execChain.front();
                if (scope.functionNode && scope.functionNode->getExecOutput() && first->getExecInput())
                    nodeView->addLink(nodeGraphLinks, scope.functionNode->getExecOutput()->getId(), first->getExecInput()->getId());
                linkExecChain(scope.execChain);
            }
            continue;
        }

        throw std::runtime_error("Unsupported Lua statement: " + line);
    }

    flushPendingGenericObjects();

    if (!functionStack.empty())
        throw std::runtime_error("Unclosed function block in Lua");

    if (!topLevelExec.empty())
        linkExecChain(topLevelExec);
}
