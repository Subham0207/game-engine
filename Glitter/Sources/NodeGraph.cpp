//
// Created by subha on 15-03-2026.
//

#include "../Headers/NodeGraph/NodeGraph.hpp"
#include <imnodes.h>
#include <imgui.h>

#include "imgui_internal.h"

NodeGraph::NodeGraph()
    : nextNodeId(0), nextCommentId(0), editorSpace(), nodesView(nodes), commentsView(comments), contextMenu()
{
}

void NodeGraph::addNode(const std::string& name, float x, float y)
{
    NodeGraphNode n(nextNodeId++, name, x, y);
    n.setSpawnPosScreen(ImVec2(x, y));
    nodes.emplace_back(std::move(n));
}

void NodeGraph::addComment(const ImVec2& posGrid)
{
    CommentBox c;
    c.id = nextCommentId++;
    c.posGrid = posGrid;
    comments.push_back(c);
}

void NodeGraph::drawUI()
{
    ImGui::Begin("node editor");

    ImNodes::BeginNodeEditor();

    // Capture stable mapping between grid-space and screen-space for this frame.
    editorSpace = NodeGraphEditorSpace::CaptureFromCurrentEditor();

    // Visual layering policy (extensible):
    // - Background elements (comments) first
    // - Nodes next
    // - Foreground overlays / context menu last
    //
    // NOTE: Comments may still query node rects for input gating. Those queries are
    // only safe for nodes that have already been submitted at least once.
    commentsView.draw(editorSpace, nodes);
    nodesView.draw();

    // Context menu last (so it can use the captured editorSpace for spawn conversion).
    contextMenu.draw(
        editorSpace,
        [&](const std::string& base, float x, float y) {
            addNode(base + " " + std::to_string(nextNodeId), x, y);
        },
        [&](const ImVec2& gridPos) { addComment(gridPos); });

    ImNodes::EndNodeEditor();

    ImGui::End();
}








