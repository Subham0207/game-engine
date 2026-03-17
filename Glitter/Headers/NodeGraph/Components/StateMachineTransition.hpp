//
// Created by subha on 17-03-2026.
//

#ifndef GLITTER_NODEGRAPH_STATEMACHINETRANSITION_HPP
#define GLITTER_NODEGRAPH_STATEMACHINETRANSITION_HPP

#include <string>

// Which side of the state node a transition connects to.
// This enables "start/end from any direction" by exposing one pin per side.
enum class StateMachinePortSide : int
{
    North = 0,
    East,
    South,
    West
};

// Model: directed transition (link) between two state machine nodes.
// Many-to-many: multiple outgoing transitions from a state and multiple incoming transitions into a state.
struct StateMachineTransition
{
    int id = -1;

    int fromNodeId = -1;
    int toNodeId = -1;

    StateMachinePortSide fromSide = StateMachinePortSide::East;
    StateMachinePortSide toSide = StateMachinePortSide::West;

    // Endpoint attachment points on the node body.
    // Stored in grid-space relative to the node rect's top-left (also in grid space).
    // This allows endpoints to be placed at any direction (including diagonals) and persisted.
    ImVec2 fromOffsetGrid{0.0f, 0.0f};
    ImVec2 toOffsetGrid{0.0f, 0.0f};

    // Prototype condition string. In the future this can be a structured expression.
    std::string condition;
};

#endif // GLITTER_NODEGRAPH_STATEMACHINETRANSITION_HPP


