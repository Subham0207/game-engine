//
// Created by subha on 26-03-2026.
//

#include "../Headers/NodeGraph/FlowScript/StatemachineFlowScript.hpp"

#include <NodeGraph/Views/NodeGraphNodesView.hpp>

#include <EngineState.hpp>
#include <Helpers/Shared.hpp>

#include <exception>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

namespace
{
    const std::string kDefaultConditionChunk =
        "return function(t)\n"
        "    return false\n"
        "end";

    std::string makeRelativeToProject(const fs::path& absolutePath)
    {
        if (!EngineState::state)
            return absolutePath.string();

        const fs::path projectDir = EngineState::state->currentActiveProjectDirectory;
        if (projectDir.empty())
            return absolutePath.string();

        std::error_code ec;
        const fs::path relative = fs::relative(absolutePath, projectDir, ec);
        if (ec || relative.empty())
            return absolutePath.string();

        return relative.string();
    }
}

StatemachineFlowScript::StatemachineFlowScript(): selectedLink(nullptr), showUI(false)
{
}

const std::string& StatemachineFlowScript::defaultConditionChunk()
{
    return kDefaultConditionChunk;
}

std::string StatemachineFlowScript::trimCopy(const std::string& value)
{
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos)
        return "";
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

std::string StatemachineFlowScript::unwrapConditionChunk(const std::string& storedCondition)
{
    const std::string trimmed = trimCopy(storedCondition);
    if (trimmed.empty())
        return "local node_0 = function(t)\n    return false\nend\n";

    if (trimmed.rfind("return function(", 0) != 0)
        return storedCondition;

    std::istringstream stream(storedCondition);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(stream, line))
        lines.push_back(line);

    if (lines.size() < 3)
        return "local node_0 = function(t)\n    return false\nend\n";

    std::vector<std::string> body(lines.begin() + 1, lines.end() - 1);
    for (int i = static_cast<int>(body.size()) - 1; i >= 0; --i)
    {
        const std::string stripped = trimCopy(body[i]);
        if (stripped.empty())
            continue;
        if (stripped.rfind("return ", 0) == 0)
            body.erase(body.begin() + i);
        break;
    }

    std::ostringstream out;
    for (const auto& bodyLine : body)
        out << bodyLine << "\n";

    const std::string unwrappedBody = trimCopy(out.str());
    if (unwrappedBody.empty())
        return "local node_0 = function(t)\n    return false\nend\n";

    return out.str();
}

std::string StatemachineFlowScript::readTextFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        return "";

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool StatemachineFlowScript::writeTextFile(const std::filesystem::path& path, const std::string& content)
{
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
        return false;

    file << content;
    return true;
}

std::filesystem::path StatemachineFlowScript::resolveScriptPath(const std::string& storedPath) const
{
    fs::path p(storedPath);
    if (p.is_absolute())
        return p;

    if (EngineState::state)
    {
        const fs::path projectDir = EngineState::state->currentActiveProjectDirectory;
        if (!projectDir.empty())
            return projectDir / p;
    }

    return p;
}

void StatemachineFlowScript::ensureSelectedLinkScriptPaths() const
{
    if (!selectedLink)
        return;

    const bool needsFlowPath = selectedLink->flowScriptPath.empty();
    const bool needsLuaPath = selectedLink->luaScriptPath.empty();
    if (!needsFlowPath && !needsLuaPath)
        return;

    fs::path baseDir = "Assets/FlowScripts";
    if (EngineState::state)
    {
        const fs::path projectDir = EngineState::state->currentActiveProjectDirectory;
        if (!projectDir.empty())
            baseDir = projectDir / "Assets" / "FlowScripts";
    }

    const std::string fileStem = Shared::uuid();
    const fs::path flowAbsolutePath = baseDir / (fileStem + ".flowscript");
    const fs::path luaAbsolutePath = baseDir / (fileStem + ".lua");

    if (needsFlowPath)
        selectedLink->flowScriptPath = makeRelativeToProject(flowAbsolutePath);
    if (needsLuaPath)
        selectedLink->luaScriptPath = makeRelativeToProject(luaAbsolutePath);
}

void StatemachineFlowScript::ensureContextNode()
{
    if (contextMembers.empty())
        return;

    for (const auto& node : nodes)
    {
        if (node && node->name() == "GenericType(t)")
            return;
    }

    auto* nodeView = views.findView<NodeGraphNodesView>();
    if (!nodeView)
        return;

    nodeView->addGenericTypeNode(nodes, "t", contextMembers, ImVec2(120.0f, 120.0f));
}

void StatemachineFlowScript::ensureContextInputConnection()
{
    NodeGraphNode* functionNode = nullptr;
    NodeGraphNode* contextNode = nullptr;

    for (const auto& node : nodes)
    {
        if (!node)
            continue;

        if (!functionNode && node->type() == NodeTypes::Function)
            functionNode = node.get();
        if (!contextNode && node->name() == "GenericType(t)")
            contextNode = node.get();
    }

    if (!functionNode || !contextNode || contextNode->inputs().empty() || functionNode->outputs().empty())
        return;

    int tOutputAttrId = functionNode->outputs()[0].getId();
    for (const auto& output : functionNode->outputs())
    {
        if (output.getName() == "t")
        {
            tOutputAttrId = output.getId();
            break;
        }
    }

    auto* nodeView = views.findView<NodeGraphNodesView>();
    if (!nodeView)
        return;

    nodeView->addLink(nodeGraphLinks, tOutputAttrId, contextNode->inputs()[0].getId());
}

std::string StatemachineFlowScript::wrapCompiledEditorScript(const std::string& compiledEditorScript)
{
    std::istringstream stream(compiledEditorScript);
    std::string line;
    std::vector<std::string> bodyLines;
    std::string entryFunctionName;
    const std::regex functionDeclRegex(R"(^\s*local\s+(node_\d+)\s*=\s*function\s*\()", std::regex::ECMAScript);

    while (std::getline(stream, line))
    {
        const std::string stripped = trimCopy(line);
        if (stripped.empty())
            continue;
        if (stripped.rfind("-- Generated by FlowScript::compile()", 0) == 0)
            continue;

        std::smatch match;
        if (std::regex_search(line, match, functionDeclRegex) && match.size() > 1)
            entryFunctionName = match[1].str();

        bodyLines.push_back(line);
    }

    std::ostringstream wrapped;
    wrapped << "return function(t)\n";
    for (const auto& bodyLine : bodyLines)
        wrapped << bodyLine << "\n";

    if (!entryFunctionName.empty())
        wrapped << "    return " << entryFunctionName << "(t)\n";
    else
        wrapped << "    return false\n";

    wrapped << "end";
    return wrapped.str();
}

void StatemachineFlowScript::setSelectedLink(StateMachineLink* link)
{
    clearScript();
    selectedLink = link;
    showUI = true;

    if (!selectedLink)
        return;

    ensureSelectedLinkScriptPaths();

    const fs::path flowScriptPath = resolveScriptPath(selectedLink->flowScriptPath);
    bool loadedFlowGraph = false;
    if (!flowScriptPath.empty())
        loadedFlowGraph = loadVisualScriptFromFile(flowScriptPath);

    if (!loadedFlowGraph)
    {
        const std::string editorSource = "local node_0 = function(t)\n    return false\nend\n";
        setCompiledLua(editorSource);
        try
        {
            deCompile(editorSource);
        }
        catch (const std::exception&)
        {
            clearScript();
        }
    }

    const fs::path luaScriptPath = resolveScriptPath(selectedLink->luaScriptPath);
    if (!luaScriptPath.empty())
        setCompiledLua(readTextFile(luaScriptPath));

    ensureContextNode();
    ensureContextInputConnection();
}

void StatemachineFlowScript::draw()
{
    if (!showUI || !selectedLink)
        return;

    ImGui::Begin("Statemachine flow script", &showUI);
    ImGui::TextUnformatted("Flow script");
    drawUIEmbedded();
    ImGui::End();
}

const std::string& StatemachineFlowScript::compile()
{
    FlowScript::compile();
    auto& luaCode = getCompiledLua();
    if (selectedLink)
    {
        ensureSelectedLinkScriptPaths();

        const fs::path flowPath = resolveScriptPath(selectedLink->flowScriptPath);
        if (!flowPath.empty())
            saveVisualScriptToFile(flowPath);

        const fs::path luaPath = resolveScriptPath(selectedLink->luaScriptPath);
        if (!luaPath.empty())
            writeTextFile(luaPath, wrapCompiledEditorScript(luaCode));
    }
    return luaCode;
}

