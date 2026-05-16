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
    // Node payload also includes:
    // - "type" (int: StateMachineNodeType)
    // - "resource_guid" (string)
    // - "blendspace_axis_x" (string)
    // - "blendspace_axis_y" (string)
    // - "animation_should_loop" (bool)
    // - "animation_complete_bool_field" (string)
    // - "animation_complete_bool_value" (bool)
    std::string ExportChainJson(const std::vector<StateMachineNode>& nodes,
                               const std::vector<StateMachineLink>& links,
                               int rootNodeId);

    // Loads state machine graph JSON authored by the editor.
    bool DeserializeChainJson(const std::string& filepath,
                          std::vector<StateMachineNode>& outNodes,
                          std::vector<StateMachineLink>& outLinks,
                          int& activeRootNodeId);
}

#endif // GLITTER_STATEMACHINEJSONEXPORTER_HPP

