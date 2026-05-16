//
// Created by subha on 17-03-2026.
//

#ifndef GLITTER_NODEGRAPH_STATEMACHINELINK_HPP
#define GLITTER_NODEGRAPH_STATEMACHINELINK_HPP

#include <string>

#include <imgui.h>

// Which side of the state node a link connects to.
// This enables "start/end from any direction" by exposing one pin per side.
enum class StateMachinePortSide : int
{
    North = 0,
    East,
    South,
    West
};

// Model: directed link between two state machine nodes.
// Many-to-many: multiple outgoing transitions from a state and multiple incoming transitions into a state.
struct StateMachineLink
{
    int id = -1;

    int fromNodeId = -1;
    int toNodeId = -1;

    StateMachinePortSide fromSide = StateMachinePortSide::East;
    StateMachinePortSide toSide = StateMachinePortSide::West;

    // Port attachment points on the node body.
    // Stored in grid-space relative to the node rect's top-left (also in grid space).
    // This allows ports to be placed at any direction (including diagonals) and persisted.
    ImVec2 fromOffsetGrid{0.0f, 0.0f};
    ImVec2 toOffsetGrid{0.0f, 0.0f};

    // Editor source asset and compiled runtime artifact for this transition condition.
    std::string flowScriptPath;
    std::string luaScriptPath;
};

// Backwards compatibility: older code used "Transition" terminology.
using StateMachineTransition = StateMachineLink;

#endif // GLITTER_NODEGRAPH_STATEMACHINELINK_HPP

