//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODEGRAPH_EDITORSPACE_HPP
#define GLITTER_NODEGRAPH_EDITORSPACE_HPP

#include <imnodes.h>
#include <imgui.h>

// Per-frame cached mapping between ImNodes grid-space and ImGui screen-space.
struct NodeGraphEditorSpace
{
    ImVec2 originScreen = ImVec2(0.0f, 0.0f); // screen-space position of grid (0,0)
    ImVec2 panning = ImVec2(0.0f, 0.0f);
    bool valid = false;

    static NodeGraphEditorSpace CaptureFromCurrentEditor()
    {
        NodeGraphEditorSpace c;
        c.panning = ImNodes::EditorContextGetPanning();
        const ImVec2 editorTopLeftScreen = ImGui::GetCursorScreenPos();
        c.originScreen = {editorTopLeftScreen.x + c.panning.x, editorTopLeftScreen.y + c.panning.y};
        c.valid = true;
        return c;
    }
};

inline ImVec2 NodeGraphGridToScreen(const NodeGraphEditorSpace& c, const ImVec2& grid)
{
    return {c.originScreen.x + grid.x, c.originScreen.y + grid.y};
}

inline ImVec2 NodeGraphScreenToGrid(const NodeGraphEditorSpace& c, const ImVec2& screen)
{
    return {screen.x - c.originScreen.x, screen.y - c.originScreen.y};
}

#endif //GLITTER_NODEGRAPH_EDITORSPACE_HPP

