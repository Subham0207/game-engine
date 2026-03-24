//
// Created by subha on 24-03-2026.
//

#include "../Headers/NodeGraph/StateMachineJsonExporter.hpp"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>

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

            json += "    { \"id\": ";
            json += std::to_string(id);
            json += ", \"name\": \"";
            json += escapeJson(name);
            json += "\" }";
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
}

