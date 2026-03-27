//
// Created by subha on 24-03-2026.
//

#include "../Headers/NodeGraph/StateMachineJsonExporter.hpp"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <iostream>
#include <boost/json.hpp>
namespace json = boost::json;
namespace
{
    StateMachineNodeType ParseNodeType(const json::value& v)
    {
        if (!v.is_number())
            return StateMachineNodeType::None;

        const int raw = json::value_to<int>(v);
        switch (raw)
        {
        case static_cast<int>(StateMachineNodeType::Blendspace):
            return StateMachineNodeType::Blendspace;
        case static_cast<int>(StateMachineNodeType::Animation):
            return StateMachineNodeType::Animation;
        default:
            return StateMachineNodeType::None;
        }
    }

    ImVec2 ParseVec2(const json::value& v)
    {
        // Current writer format: { "x": ..., "y": ... }
        if (v.is_object())
        {
            const auto& obj = v.as_object();
            const auto xIt = obj.find("x");
            const auto yIt = obj.find("y");
            if (xIt != obj.end() && yIt != obj.end() &&
                xIt->value().is_number() && yIt->value().is_number())
            {
                return ImVec2(
                    json::value_to<float>(xIt->value()),
                    json::value_to<float>(yIt->value())
                );
            }
        }

        // Backward compatibility for legacy format: [x, y]
        if (!v.is_array())
            return ImVec2(0.0f, 0.0f);

        const auto& arr = v.as_array();
        if (arr.size() != 2 || !arr[0].is_number() || !arr[1].is_number())
            return ImVec2(0.0f, 0.0f);

        return ImVec2(
            json::value_to<float>(arr[0]),
            json::value_to<float>(arr[1])
        );
    }
}
namespace
{
    static std::string escapeJson(const std::string& s)
    {
        std::string out;
        out.reserve(s.size() + 8);
        for (char c : s)
        {
            switch (c)
            {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20)
                {
                    // control chars -> \u00XX
                    static const char* hex = "0123456789ABCDEF";
                    out += "\\u00";
                    out += hex[(c >> 4) & 0xF];
                    out += hex[c & 0xF];
                }
                else
                {
                    out += c;
                }
                break;
            }
        }
        return out;
    }

    static int findNodeIndexById(const std::vector<StateMachineNode>& nodes, int id)
    {
        for (size_t i = 0; i < nodes.size(); ++i)
            if (nodes[i].id == id)
                return static_cast<int>(i);
        return -1;
    }

    static void appendVec2(std::string& json, const ImVec2& v)
    {
        json += "{ \"x\": ";
        json += std::to_string(v.x);
        json += ", \"y\": ";
        json += std::to_string(v.y);
        json += " }";
    }
}

namespace StateMachineJsonExporter
{
    std::string ExportChainJson(const std::vector<StateMachineNode>& nodes,
                               const std::vector<StateMachineLink>& links,
                               int rootNodeId)
    {
        if (findNodeIndexById(nodes, rootNodeId) < 0)
            return "{}";

        // Build adjacency for deterministic traversal.
        std::unordered_map<int, std::vector<const StateMachineLink*>> out;
        out.reserve(nodes.size());
        for (const auto& l : links)
            out[l.fromNodeId].push_back(&l);
        for (auto& [_, v] : out)
        {
            std::sort(v.begin(), v.end(), [](const StateMachineLink* a, const StateMachineLink* b) {
                if (a->toNodeId != b->toNodeId) return a->toNodeId < b->toNodeId;
                return a->id < b->id;
            });
        }

        // BFS from root.
        std::queue<int> q;
        std::unordered_set<int> visited;
        visited.reserve(nodes.size());
        q.push(rootNodeId);
        visited.insert(rootNodeId);

        std::vector<int> reachableNodes;
        reachableNodes.reserve(nodes.size());

        std::vector<const StateMachineLink*> reachableLinks;
        reachableLinks.reserve(links.size());

        while (!q.empty())
        {
            const int cur = q.front();
            q.pop();
            reachableNodes.push_back(cur);

            auto it = out.find(cur);
            if (it == out.end())
                continue;

            for (const StateMachineLink* l : it->second)
            {
                reachableLinks.push_back(l);
                const int nxt = l->toNodeId;
                if (visited.insert(nxt).second)
                    q.push(nxt);
            }
        }

        std::sort(reachableNodes.begin(), reachableNodes.end());
        std::sort(reachableLinks.begin(), reachableLinks.end(), [](const StateMachineLink* a, const StateMachineLink* b) {
            if (a->fromNodeId != b->fromNodeId) return a->fromNodeId < b->fromNodeId;
            if (a->toNodeId != b->toNodeId) return a->toNodeId < b->toNodeId;
            return a->id < b->id;
        });

        // Build JSON.
        std::string json;
        json.reserve(1024);
        json += "{\n";
        json += "  \"root\": ";
        json += std::to_string(rootNodeId);
        json += ",\n";

        json += "  \"nodes\": [\n";
        for (size_t i = 0; i < reachableNodes.size(); ++i)
        {
            const int id = reachableNodes[i];
            const int idx = findNodeIndexById(nodes, id);
            const std::string name = (idx >= 0) ? nodes[idx].name : std::string();
            const ImVec2 pos = (idx >= 0) ? nodes[idx].spawnPosScreen : ImVec2(0.0f, 0.0f);
            const StateMachineNodeType type = (idx >= 0) ? nodes[idx].type : StateMachineNodeType::None;
            const std::string resourceGuid = (idx >= 0) ? nodes[idx].resourceGuid : std::string();

            json += "    { \"id\": ";
            json += std::to_string(id);
            json += ", \"name\": \"";
            json += escapeJson(name);
            json += "\", \"pos\": ";
            appendVec2(json, pos);
            json += ", \"type\": ";
            json += std::to_string(static_cast<int>(type));
            json += ", \"resource_guid\": \"";
            json += escapeJson(resourceGuid);
            json += "\"";
            json += " }";
            if (i + 1 < reachableNodes.size())
                json += ",";
            json += "\n";
        }
        json += "  ],\n";

        json += "  \"transitions\": [\n";
        for (size_t i = 0; i < reachableLinks.size(); ++i)
        {
            const StateMachineLink* l = reachableLinks[i];
            json += "    { \"id\": ";
            json += std::to_string(l->id);
            json += ", \"from\": ";
            json += std::to_string(l->fromNodeId);
            json += ", \"to\": ";
            json += std::to_string(l->toNodeId);
            json += ", \"from_side\": ";
            json += std::to_string(static_cast<int>(l->fromSide));
            json += ", \"to_side\": ";
            json += std::to_string(static_cast<int>(l->toSide));
            json += ", \"from_offset_grid\": ";
            appendVec2(json, l->fromOffsetGrid);
            json += ", \"to_offset_grid\": ";
            appendVec2(json, l->toOffsetGrid);
            json += ", \"condition\": \"";
            json += escapeJson(l->condition);
            json += "\" }";
            if (i + 1 < reachableLinks.size())
                json += ",";
            json += "\n";
        }
        json += "  ]\n";

        json += "}";
        return json;
    }

    bool DeserializeChainJson(const std::string& filePath,
                          std::vector<StateMachineNode>& outNodes,
                          std::vector<StateMachineLink>& outLinks,
                          int& activeRootNodeId)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cout << "Failed to open file: " << filePath << std::endl;
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        auto jsonStr = buffer.str();
        if (jsonStr.empty())
        {
            std::cout << "File is empty: " << filePath << std::endl;
            return false;
        }

        outNodes.clear();
        outLinks.clear();
        activeRootNodeId = -1;

        try
        {
            json::value rootVal = json::parse(jsonStr);

            if (!rootVal.is_object())
                return false;

            const json::object& rootObj = rootVal.as_object();

            // Root node
            if (auto it = rootObj.find("root"); it != rootObj.end())
            {
                activeRootNodeId = json::value_to<int>(it->value());
            }
            else
            {
                return false;
            }

            // Nodes
            if (auto it = rootObj.find("nodes"); it != rootObj.end() && it->value().is_array())
            {
                const auto& nodesArr = it->value().as_array();

                for (const auto& nVal : nodesArr)
                {
                    if (!nVal.is_object())
                        continue;

                    const auto& n = nVal.as_object();

                    StateMachineNode node{};

                    if (auto f = n.find("id"); f != n.end())
                        node.id = json::value_to<int>(f->value());

                    if (auto f = n.find("name"); f != n.end())
                        node.name = json::value_to<std::string>(f->value());

                    if (auto f = n.find("pos"); f != n.end())
                    {
                        node.spawnPosScreen = ParseVec2(f->value());
                        node.hasSpawn = true;
                    }
                    else
                        node.spawnPosScreen = ImVec2(0.0f, 0.0f);

                    if (auto f = n.find("type"); f != n.end())
                        node.type = ParseNodeType(f->value());

                    if (auto f = n.find("resource_guid"); f != n.end() && f->value().is_string())
                        node.resourceGuid = json::value_to<std::string>(f->value());

                    outNodes.push_back(std::move(node));
                }
            }

            // Transitions (links)
            if (auto it = rootObj.find("transitions"); it != rootObj.end() && it->value().is_array())
            {
                const auto& transArr = it->value().as_array();

                for (const auto& tVal : transArr)
                {
                    if (!tVal.is_object())
                        continue;

                    const auto& t = tVal.as_object();

                    StateMachineLink link{};

                    if (auto f = t.find("id"); f != t.end())
                        link.id = json::value_to<int>(f->value());

                    if (auto f = t.find("from"); f != t.end())
                        link.fromNodeId = json::value_to<int>(f->value());

                    if (auto f = t.find("to"); f != t.end())
                        link.toNodeId = json::value_to<int>(f->value());

                    if (auto f = t.find("from_side"); f != t.end())
                        link.fromSide = static_cast<StateMachinePortSide>(json::value_to<int>(f->value()));

                    if (auto f = t.find("to_side"); f != t.end())
                        link.toSide = static_cast<StateMachinePortSide>(json::value_to<int>(f->value()));

                    if (auto f = t.find("from_offset_grid"); f != t.end())
                        link.fromOffsetGrid = ParseVec2(f->value());

                    if (auto f = t.find("to_offset_grid"); f != t.end())
                        link.toOffsetGrid = ParseVec2(f->value());

                    if (auto f = t.find("condition"); f != t.end())
                        link.condition = json::value_to<std::string>(f->value());

                    outLinks.push_back(std::move(link));
                }
            }

            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
}
