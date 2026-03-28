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
    struct AttrInfo
    {
        NodeGraphNode* node = nullptr;
        int index = -1;
        bool isInput = false;
        NodeAttributeType type = NodeAttributeType::PIN;
    };

    std::unordered_map<int, AttrInfo> attrInfo;
    std::unordered_map<int, int> dataInputToOutput;
    std::unordered_map<int, int> execInputToOutput;
    std::unordered_map<NodeGraphNode*, std::unordered_set<NodeGraphNode*>> deps;
    std::unordered_map<NodeGraphNode*, std::vector<NodeGraphNode*>> outgoing;
    std::unordered_map<NodeGraphNode*, int> indegree;
    std::vector<NodeGraphNode*> nodeList; // we already have this type of object available here. It's called nodes.
    nodeList.reserve(nodes.size());

    for (auto& nodePtr : nodes)
    {
        NodeGraphNode* node = nodePtr.get();
        nodeList.push_back(node);
        indegree[node] = 0;
        deps[node];
        outgoing[node];

        if (node->hasExecInput())
        {
            auto* execInput = node->getExecInput();
            attrInfo[execInput->getId()] = { node, -1, true, execInput->getType() };
        }
        if (node->hasExecOutput())
        {
            auto* execOutput = node->getExecOutput();
            attrInfo[execOutput->getId()] = { node, -1, false, execOutput->getType() };
        }

        auto& inputs = node->inputs();
        for (int i = 0; i < static_cast<int>(inputs.size()); ++i)
            attrInfo[inputs[i].getId()] = { node, i, true, inputs[i].getType() };

        auto& outputs = node->outputs();
        for (int i = 0; i < static_cast<int>(outputs.size()); ++i)
            attrInfo[outputs[i].getId()] = { node, i, false, outputs[i].getType() };
    }

    for (const auto& link : nodeGraphLinks)
    {
        auto startIt = attrInfo.find(link.startAttr());
        auto endIt = attrInfo.find(link.endAttr());
        if (startIt == attrInfo.end() || endIt == attrInfo.end())
            continue;
        if (!endIt->second.isInput) // In addition, also check startIt is Input.
            continue;

        if (startIt->second.type == NodeAttributeType::ExecutionFlowOutPin
            && endIt->second.type == NodeAttributeType::ExecutionFlowInPin)
        {
            execInputToOutput[link.endAttr()] = link.startAttr();
            continue;
        }

        if (endIt->second.type == NodeAttributeType::PIN
            && (startIt->second.type == NodeAttributeType::PIN || startIt->second.type == NodeAttributeType::FIELD))
        {
            dataInputToOutput[link.endAttr()] = link.startAttr();
        }
    }

    for (auto* node : nodeList)
    {
        for (const auto& inputAttr : node->inputs())
        {
            if (inputAttr.getType() != NodeAttributeType::PIN)
                continue;

            auto inputIt = dataInputToOutput.find(inputAttr.getId());
            if (inputIt == dataInputToOutput.end())
                continue;

            auto outputIt = attrInfo.find(inputIt->second);
            if (outputIt == attrInfo.end())
                continue;

            NodeGraphNode* srcNode = outputIt->second.node;
            if (srcNode == node)
                continue;

            if (deps[node].insert(srcNode).second)
            {
                indegree[node]++;
                outgoing[srcNode].push_back(node);
            }
        }
    }

    std::queue<NodeGraphNode*> ready;
    for (auto* node : nodeList)
    {
        if (indegree[node] == 0)
            ready.push(node);
    }

    std::vector<NodeGraphNode*> ordered;
    ordered.reserve(nodeList.size());
    while (!ready.empty())
    {
        NodeGraphNode* node = ready.front();
        ready.pop();
        ordered.push_back(node);

        for (auto* next : outgoing[node])
        {
            if (--indegree[next] == 0)
                ready.push(next);
        }
    }

    if (ordered.size() != nodeList.size())
    {
        for (auto* node : nodeList)
        {
            if (std::find(ordered.begin(), ordered.end(), node) == ordered.end())
                ordered.push_back(node);
        }
    }

    std::unordered_map<NodeGraphNode*, std::string> nodeVar;
    nodeVar.reserve(nodeList.size());
    for (auto* node : nodeList)
        nodeVar[node] = "node_" + std::to_string(node->id());

    bool hasExplicitReturn = false;
    for (auto* node : nodeList)
    {
        if (node->name() == "Return")
        {
            hasExplicitReturn = true;
            break;
        }
    }

    std::unordered_set<NodeGraphNode*> referencedOutputs;
    for (const auto& link : nodeGraphLinks)
    {
        auto startIt = attrInfo.find(link.startAttr());
        if (startIt == attrInfo.end())
            continue;
        if (startIt->second.type == NodeAttributeType::ExecutionFlowOutPin)
            continue;
        referencedOutputs.insert(startIt->second.node);
    }

    auto resolveInputExpr = [&](const NodeGraphComponents::Node::Attribute& inputAttr)
    {
        auto inputIt = dataInputToOutput.find(inputAttr.getId());
        if (inputIt == dataInputToOutput.end())
            return std::string("0");
        auto outputIt = attrInfo.find(inputIt->second);
        if (outputIt == attrInfo.end() || !outputIt->second.node)
            return std::string("0");
        return nodeVar[outputIt->second.node];
    };

    std::unordered_map<NodeGraphNode*, std::vector<NodeGraphNode*>> execOutgoing;
    std::unordered_map<NodeGraphNode*, int> execIndegree;
    std::unordered_set<NodeGraphNode*> execNodes;
    for (auto* node : nodeList)
    {
        if (node->hasExecInput() || node->hasExecOutput())
        {
            execNodes.insert(node);
            execIndegree[node] = 0;
            execOutgoing[node];
        }
    }

    for (const auto& link : execInputToOutput)
    {
        auto outputIt = attrInfo.find(link.second);
        auto inputIt = attrInfo.find(link.first);
        if (outputIt == attrInfo.end() || inputIt == attrInfo.end())
            continue;
        NodeGraphNode* srcNode = outputIt->second.node;
        NodeGraphNode* dstNode = inputIt->second.node;
        if (!srcNode || !dstNode || srcNode == dstNode)
            continue;
        execOutgoing[srcNode].push_back(dstNode);
        execIndegree[dstNode]++;
    }

    auto emitIndent = [](int depth)
    {
        return std::string(static_cast<size_t>(depth) * 4u, ' ');
    };

    std::unordered_set<NodeGraphNode*> visitedExec;
    auto emitStatement = [&](std::ostringstream& stream, NodeGraphNode* node, int indent)
    {
        const std::string& nodeName = node->name();
        const std::string& varName = nodeVar[node];
        const std::string pad = emitIndent(indent);

        if (nodeName == "Add")
        {
            std::string expr = "0";
            if (!node->inputs().empty())
            {
                expr = resolveInputExpr(node->inputs()[0]);
                for (size_t i = 1; i < node->inputs().size(); ++i)
                    expr += " + " + resolveInputExpr(node->inputs()[i]);
                expr = "(" + expr + ")";
            }
            stream << pad << "local " << varName << " = " << expr << "\n";
        }
        else if (nodeName == "Subtract")
        {
            std::string expr = "0";
            if (!node->inputs().empty())
            {
                expr = resolveInputExpr(node->inputs()[0]);
                for (size_t i = 1; i < node->inputs().size(); ++i)
                    expr += " - " + resolveInputExpr(node->inputs()[i]);
                expr = "(" + expr + ")";
            }
            stream << pad << "local " << varName << " = " << expr << "\n";
        }
        else if (nodeName == "GreaterThan")
        {
            std::string expr = "0";
            if (node->inputs().size() >= 2)
            {
                const std::string left = resolveInputExpr(node->inputs()[0]);
                const std::string right = resolveInputExpr(node->inputs()[1]);
                expr = "(" + left + " > " + right + ")";
            }
            stream << pad << "local " << varName << " = " << expr << "\n";
        }
        else if (nodeName == "EqualsTo")
        {
            std::string expr = "false";
            if (node->inputs().size() >= 2)
            {
                const std::string left = resolveInputExpr(node->inputs()[0]);
                const std::string right = resolveInputExpr(node->inputs()[1]);
                expr = "(" + left + " == " + right + ")";
            }
            stream << pad << "local " << varName << " = " << expr << "\n";
        }
        else if (nodeName == "NotEqualsTo")
        {
            std::string expr = "false";
            if (node->inputs().size() >= 2)
            {
                const std::string left = resolveInputExpr(node->inputs()[0]);
                const std::string right = resolveInputExpr(node->inputs()[1]);
                expr = "(" + left + " ~= " + right + ")";
            }
            stream << pad << "local " << varName << " = " << expr << "\n";
        }
        else if (nodeName == "Function")
        {
            stream << pad << "local " << varName << " = function()\n";
        }
        else if (nodeName == "Print")
        {
            std::string expr = "0";
            if (!node->inputs().empty())
                expr = resolveInputExpr(node->inputs()[0]);
            stream << pad << "print(\"[LUA]\", " << expr << ")\n";
        }
        else if (nodeName == "Return")
        {
            std::string expr = "0";
            if (!node->inputs().empty())
                expr = resolveInputExpr(node->inputs()[0]);
            stream << pad << "return " << expr << "\n";
        }
        else
        {
            stream << pad << "local " << varName << " = 0 -- unsupported node: " << nodeName << "\n";
        }
    };

    std::function<void(std::ostringstream&, NodeGraphNode*, int)> emitExecChain;
    emitExecChain = [&](std::ostringstream& stream, NodeGraphNode* node, int indent)
    {
        if (!node || visitedExec.count(node) != 0)
            return;
        visitedExec.insert(node);

        const bool isFunction = node->name() == "Function";
        if (isFunction)
        {
            emitStatement(stream, node, indent);
            for (auto* next : execOutgoing[node])
                emitExecChain(stream, next, indent + 1);
            stream << emitIndent(indent) << "end\n";
            return;
        }

        emitStatement(stream, node, indent);

        if (node->name() == "Return")
            return;

        for (auto* next : execOutgoing[node])
            emitExecChain(stream, next, indent);
    };

    std::ostringstream out;
    out << "-- Generated by FlowScript::compile()\n";

    for (auto* node : ordered)
    {
        if (node->hasExecInput() || node->hasExecOutput())
            continue;

        const std::string& nodeName = node->name();
        const std::string& varName = nodeVar[node];

        if (nodeName == "Integer")
        {
            std::string value = "0";
            if (!node->outputs().empty())
                value = parseNumberOrDefault(node->outputs()[0].getValueBuff());
            out << "local " << varName << " = " << value << "\n";
        }
        else if (nodeName == "Boolean")
        {
            std::string value = "false";
            if (!node->outputs().empty())
                value = parseBooleanOrDefault(node->outputs()[0].getValueBuff());
            out << "local " << varName << " = " << value << "\n";
        }
        else if (nodeName == "EqualsTo")
        {
            std::string expr = "false";
            if (node->inputs().size() >= 2)
            {
                const std::string left = resolveInputExpr(node->inputs()[0]);
                const std::string right = resolveInputExpr(node->inputs()[1]);
                expr = "(" + left + " == " + right + ")";
            }
            out << "local " << varName << " = " << expr << "\n";
        }
        else if (nodeName == "NotEqualsTo")
        {
            std::string expr = "false";
            if (node->inputs().size() >= 2)
            {
                const std::string left = resolveInputExpr(node->inputs()[0]);
                const std::string right = resolveInputExpr(node->inputs()[1]);
                expr = "(" + left + " ~= " + right + ")";
            }
            out << "local " << varName << " = " << expr << "\n";
        }
        else
        {
            out << "local " << varName << " = 0 -- unsupported node: " << nodeName << "\n";
        }
    }

    for (auto* node : nodeList)
    {
        if ((node->hasExecInput() || node->hasExecOutput()) && execIndegree[node] == 0)
            emitExecChain(out, node, 0);
    }

    for (auto* node : nodeList)
    {
        if (execNodes.count(node) && visitedExec.count(node) == 0)
            emitExecChain(out, node, 0);
    }

    std::vector<std::string> terminalVars;
    for (auto* node : ordered)
    {
        if (referencedOutputs.find(node) == referencedOutputs.end() && !node->outputs().empty())
            terminalVars.push_back(nodeVar[node]);
    }

    if (!terminalVars.empty() && !hasExplicitReturn)
    {
        out << "return ";
        if (terminalVars.size() == 1)
        {
            out << terminalVars.front() << "\n";
        }
        else
        {
            out << "{ ";
            for (size_t i = 0; i < terminalVars.size(); ++i)
            {
                if (i > 0)
                    out << ", ";
                out << terminalVars[i];
            }
            out << " }\n";
        }
    }

    compiledLua = out.str();
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
        std::string line = trimCopy(rawLine);
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
