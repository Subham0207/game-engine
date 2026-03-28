//
// Created by subha on 28-03-2026.
//

#ifndef GLITTER_GRAPHINDEXLOOKUP_HPP
#define GLITTER_GRAPHINDEXLOOKUP_HPP
#pragma once
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "NodeGraph/Components/NodeGraphNode.hpp"
#include "NodeGraph/Components/NodeGraphNodeLink.hpp"

namespace Flowscript::Compile
{
    struct PinInfo
    {
        NodeGraphNode* node = nullptr;
        int index = -1;
        bool isInput = false;
        NodeAttributeType type = NodeAttributeType::PIN;
    };

    struct GraphIndex
    {
        std::unordered_map<int, PinInfo> pinInfoById;
        std::unordered_map<int, int> dataSourcePinByInputPin;
        std::unordered_map<int, int> execSourcePinByInputPin;

        std::unordered_map<NodeGraphNode*, std::vector<NodeGraphNode*>> execOutgoing;
        std::unordered_map<NodeGraphNode*, int> execIndegree;

        std::unordered_map<NodeGraphNode*, std::unordered_set<NodeGraphNode*>> dataDeps;
        std::unordered_map<NodeGraphNode*, std::vector<NodeGraphNode*>> dataOutgoing;
        std::unordered_map<NodeGraphNode*, int> dataIndegree;

        std::vector<NodeGraphNode*> allNodes;
    };

    inline const PinInfo* FindPinInfo(const GraphIndex& graph, const int pinId)
    {
        const auto it = graph.pinInfoById.find(pinId);
        if (it == graph.pinInfoById.end())
            return nullptr;
        return &it->second;
    }

    inline NodeGraphNode* GetDataSourceNode(const GraphIndex& graph, const int inputPinId)
    {
        const auto sourcePinIt = graph.dataSourcePinByInputPin.find(inputPinId);
        if (sourcePinIt == graph.dataSourcePinByInputPin.end())
            return nullptr;

        const auto* sourcePin = FindPinInfo(graph, sourcePinIt->second);
        return sourcePin ? sourcePin->node : nullptr;
    }

    inline const PinInfo* GetDataSourcePinInfo(const GraphIndex& graph, const int inputPinId)
    {
        const auto sourcePinIt = graph.dataSourcePinByInputPin.find(inputPinId);
        if (sourcePinIt == graph.dataSourcePinByInputPin.end())
            return nullptr;
        return FindPinInfo(graph, sourcePinIt->second);
    }

    inline std::vector<NodeGraphNode*> GetExecNextNodes(const GraphIndex& graph, NodeGraphNode* node)
    {
        if (!node)
            return {};
        const auto it = graph.execOutgoing.find(node);
        if (it == graph.execOutgoing.end())
            return {};
        return it->second;
    }

    inline std::vector<NodeGraphNode*> TopoSortDataNodes(const GraphIndex& graph)
    {
        std::unordered_map<NodeGraphNode*, int> indegree = graph.dataIndegree;
        std::vector<NodeGraphNode*> ready;
        ready.reserve(graph.allNodes.size());
        for (auto* node : graph.allNodes)
        {
            if (indegree[node] == 0)
                ready.push_back(node);
        }

        std::vector<NodeGraphNode*> ordered;
        ordered.reserve(graph.allNodes.size());
        while (!ready.empty())
        {
            NodeGraphNode* node = ready.back();
            ready.pop_back();
            ordered.push_back(node);

            const auto outIt = graph.dataOutgoing.find(node);
            if (outIt == graph.dataOutgoing.end())
                continue;

            for (auto* next : outIt->second)
            {
                if (--indegree[next] == 0)
                    ready.push_back(next);
            }
        }

        if (ordered.size() != graph.allNodes.size())
        {
            for (auto* node : graph.allNodes)
            {
                if (std::find(ordered.begin(), ordered.end(), node) == ordered.end())
                    ordered.push_back(node);
            }
        }

        return ordered;
    }

    inline GraphIndex BuildGraphIndex(const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
                                      const std::vector<NodeGraphNodeLink>& links)
    {
        GraphIndex graph;
        graph.allNodes.reserve(nodes.size());

        for (const auto& nodePtr : nodes)
        {
            NodeGraphNode* node = nodePtr.get();
            graph.allNodes.push_back(node);

            graph.dataDeps[node];
            graph.dataOutgoing[node];
            graph.dataIndegree[node] = 0;

            if (node->hasExecInput() || node->hasExecOutput())
            {
                graph.execOutgoing[node];
                graph.execIndegree[node] = 0;
            }

            if (node->hasExecInput())
            {
                auto* pin = node->getExecInput();
                graph.pinInfoById[pin->getId()] = { node, -1, true, pin->getType() };
            }
            if (node->hasExecOutput())
            {
                auto* pin = node->getExecOutput();
                graph.pinInfoById[pin->getId()] = { node, -1, false, pin->getType() };
            }

            auto& inputs = node->inputs();
            for (int i = 0; i < static_cast<int>(inputs.size()); ++i)
                graph.pinInfoById[inputs[i].getId()] = { node, i, true, inputs[i].getType() };

            auto& outputs = node->outputs();
            for (int i = 0; i < static_cast<int>(outputs.size()); ++i)
                graph.pinInfoById[outputs[i].getId()] = { node, i, false, outputs[i].getType() };
        }

        for (const auto& link : links)
        {
            const auto* start = FindPinInfo(graph, link.startAttr());
            const auto* end = FindPinInfo(graph, link.endAttr());
            if (!start || !end || !end->isInput)
                continue;

            if (start->type == NodeAttributeType::ExecutionFlowOutPin
                && end->type == NodeAttributeType::ExecutionFlowInPin)
            {
                graph.execSourcePinByInputPin[link.endAttr()] = link.startAttr();
                continue;
            }

            if (end->type == NodeAttributeType::PIN
                && (start->type == NodeAttributeType::PIN || start->type == NodeAttributeType::FIELD))
            {
                graph.dataSourcePinByInputPin[link.endAttr()] = link.startAttr();
            }
        }

        for (const auto& [execInputPin, execOutputPin] : graph.execSourcePinByInputPin)
        {
            const auto* srcPin = FindPinInfo(graph, execOutputPin);
            const auto* dstPin = FindPinInfo(graph, execInputPin);
            if (!srcPin || !dstPin || !srcPin->node || !dstPin->node)
                continue;
            if (srcPin->node == dstPin->node)
                continue;

            graph.execOutgoing[srcPin->node].push_back(dstPin->node);
            graph.execIndegree[dstPin->node] += 1;
        }

        for (auto* node : graph.allNodes)
        {
            for (const auto& input : node->inputs())
            {
                if (input.getType() != NodeAttributeType::PIN)
                    continue;

                const auto* srcPin = GetDataSourcePinInfo(graph, input.getId());
                if (!srcPin || !srcPin->node || srcPin->node == node)
                    continue;

                if (graph.dataDeps[node].insert(srcPin->node).second)
                {
                    graph.dataIndegree[node] += 1;
                    graph.dataOutgoing[srcPin->node].push_back(node);
                }
            }
        }

        return graph;
    }
}

#endif //GLITTER_GRAPHINDEXLOOKUP_HPP

