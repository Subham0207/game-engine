//
// Created by subha on 24-03-2026.
//

#ifndef GLITTER_STATEMACHINEJSONEXPORTER_HPP
#define GLITTER_STATEMACHINEJSONEXPORTER_HPP

#include <string>
#include <vector>

#include "Components/StateMachineNode.hpp"
#include "Components/StateMachineLink.hpp"

// Minimal JSON exporter for the editor state-machine model.
//
// "Chain" here means the reachable graph starting from a root state
// (typically the currently active state), following directed links.
//
// Output is deterministic: nodes and transitions are sorted by id.
namespace StateMachineJsonExporter
{
    // Export the graph reachable starting from `rootNodeId`.
    // If `rootNodeId` is not found, returns an empty JSON object string: "{}".
    std::string ExportChainJson(const std::vector<StateMachineNode>& nodes,
                               const std::vector<StateMachineLink>& links,
                               int rootNodeId);

    bool DeserializeChainJson(const std::string& filepath,
                          std::vector<StateMachineNode>& outNodes,
                          std::vector<StateMachineLink>& outLinks,
                          int& activeRootNodeId);
}

#endif // GLITTER_STATEMACHINEJSONEXPORTER_HPP

