//
// Created by subha on 17-03-2026.
//

#ifndef GLITTER_NODEGRAPH_STATEMACHINENODE_HPP
#define GLITTER_NODEGRAPH_STATEMACHINENODE_HPP

#include <string>
#include <imgui.h>

enum class StateMachineNodeType : int
{
    Blendspace,
    Animation,
    None
};

// Model: a state machine node in the NodeGraph editor.
// Persistent data only; UI state belongs in the view.
struct StateMachineNode
{
    int id = -1;
    std::string name;

    // Stored in screen-space because ImNodes works in screen-space.
    // The view is responsible for calling ImNodes::SetNodeScreenSpacePos once.
    ImVec2 spawnPosScreen{0.0f, 0.0f};
    bool hasSpawn = false;
    bool posSet = false;

    /*
     * type is blendspace then resource guid is blendspace guid.
     * type is animation then resource guid is animation guid.
     */
    StateMachineNodeType type = StateMachineNodeType::None;
    std::string resourceGuid;

    // Blendspace driver variable names from the reflected context type T.
    // Stored by name so bindings stay stable across sessions.
    std::string blendspaceAxisXField;
    std::string blendspaceAxisYField;

    // Runtime-ish flags for the editor prototype.
    // "Active" here means "currently active state" inside the editor simulation.
    bool active = false;
};

#endif // GLITTER_NODEGRAPH_STATEMACHINENODE_HPP

