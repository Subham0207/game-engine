//
// Created by subha on 15-03-2026.
//

#include "../Headers/NodeGraph/NodeGraph.hpp"
#include <imnodes.h>
#include <imgui.h>

#include "imgui_internal.h"

namespace
{
// These helpers are cached per-frame from within NodeGraph::drawUI() after
// ImNodes::BeginNodeEditor(). They provide consistent conversion between
// ImNodes grid-space and ImGui screen-space.
struct EditorSpaceCache
{
    ImVec2 originScreen = ImVec2(0.0f, 0.0f); // screen-space position of grid (0,0)
    ImVec2 panning = ImVec2(0.0f, 0.0f);
    bool valid = false;
};

inline ImVec2 GridToScreen(const EditorSpaceCache& c, const ImVec2& grid)
{
    return {c.originScreen.x + grid.x, c.originScreen.y + grid.y};
}

inline ImVec2 ScreenToGrid(const EditorSpaceCache& c, const ImVec2& screen)
{
    return {screen.x - c.originScreen.x, screen.y - c.originScreen.y};
}
} // namespace

// Cached per-frame in drawUI() right after ImNodes::BeginNodeEditor().
static EditorSpaceCache g_editorSpaceCache;

NodeGraph::NodeGraph()
    : nextNodeId(0), nextCommentId(0), showContextMenu(false), contextMenuX(0.0f), contextMenuY(0.0f),
      spawnPosScreen(0.0f, 0.0f), activeCommentId(-1), resizingComment(false), dragOffset(0.0f, 0.0f),
      editingCommentId(-1)
{
}

void NodeGraph::addNode(const std::string& name, float x, float y)
{
    Node node;
    node.id = nextNodeId++;
    node.name = name;
    node.x = x;
    node.y = y;
    node.positionSet = false;  // Position not yet set in ImNodes
    nodes.push_back(node);
}

void NodeGraph::addComment(const ImVec2& posGrid)
{
    CommentBox c;
    c.id = nextCommentId++;
    c.posGrid = posGrid;
    comments.push_back(c);
}

void NodeGraph::drawComments()
{
    // Draw behind nodes (call before drawNodes()).
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Coordinate conversion:
    // grid -> screen and screen -> grid. Cached per-frame in drawUI().
    // If cache isn't valid yet (shouldn't happen in normal flow), fall back to
    // local estimation.
    // Use cached origin computed immediately after ImNodes::BeginNodeEditor().
    // If cache isn't valid (unexpected), fall back to best-effort estimate.
    const EditorSpaceCache& cache = g_editorSpaceCache.valid ? g_editorSpaceCache : [&]() -> const EditorSpaceCache& {
        static EditorSpaceCache fallback;
        fallback.panning = ImNodes::EditorContextGetPanning();
        // Approximate: cursor is inside editor; originScreen should be top-left of editor minus panning.
        fallback.originScreen = ImGui::GetCursorScreenPos();
        // Screen position of grid (0,0) is (editorTopLeft + panning).
        fallback.originScreen.x += fallback.panning.x;
        fallback.originScreen.y += fallback.panning.y;
        fallback.valid = true;
        return fallback;
    }();

    auto gridToScreen = [&](const ImVec2& g) -> ImVec2 { return GridToScreen(cache, g); };
    auto screenToGrid = [&](const ImVec2& s) -> ImVec2 { return ScreenToGrid(cache, s); };

    const ImU32 fillCol = IM_COL32(255, 255, 0, 40);
    const ImU32 borderCol = IM_COL32(255, 255, 0, 140);
    const ImU32 headerCol = IM_COL32(255, 255, 0, 80);

    const float headerH = 22.0f;
    const float handle = 12.0f;
    const ImVec2 minSize(120.0f, 60.0f);

    // Basic interaction: move by dragging header, resize by dragging bottom-right handle.
    // Title text is read-only by default; double-click header to edit.
    // Evaluate hovered comment from top-most (last) to bottom.
    int hoveredIdx = -1;
    bool hoveringImNodesElement = false;
    {
        int hoveredNode = -1;
        int hoveredLink = -1;
        int hoveredPin = -1;

        // If the cursor is over any ImNodes element, comments must NOT capture
        // input; nodes should be draggable even if visually "inside" a comment.
        (void)ImNodes::IsNodeHovered(&hoveredNode);
        (void)ImNodes::IsLinkHovered(&hoveredLink);
        (void)ImNodes::IsPinHovered(&hoveredPin);

        hoveringImNodesElement = (hoveredNode != -1) || (hoveredLink != -1) || (hoveredPin != -1);
    }

    const ImVec2 mouse = ImGui::GetMousePos();
    if (ImNodes::IsEditorHovered() && !hoveringImNodesElement)
    {
        for (int i = (int)comments.size() - 1; i >= 0; --i)
        {
            const CommentBox& c = comments[i];
            const ImVec2 p0 = gridToScreen(c.posGrid);
            const ImRect r(p0, ImVec2(p0.x + c.size.x, p0.y + c.size.y));
            if (r.Contains(mouse))
            {
                hoveredIdx = i;
                break;
            }
        }
    }

    // Click outside -> stop editing
    if (editingCommentId != -1 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        bool clickedInsideEditing = false;
        for (const auto& c : comments)
        {
            if (c.id != editingCommentId) continue;
            const ImVec2 p0 = gridToScreen(c.posGrid);
            const ImRect headerRect(p0, ImVec2(p0.x + c.size.x, p0.y + headerH));
            clickedInsideEditing = headerRect.Contains(mouse);
            break;
        }
        if (!clickedInsideEditing)
        {
            editingCommentId = -1;
            ImGui::ClearActiveID();
        }
    }

    // IMPORTANT: If the mouse is over a node/link, comments must not capture
    // interactions; nodes should remain draggable even if visually "inside"
    // a comment rectangle.
    const bool allowCommentInteraction = !hoveringImNodesElement;

    // Double click header -> edit title
    if (allowCommentInteraction && hoveredIdx != -1 && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        const auto& c = comments[hoveredIdx];
        const ImVec2 p0 = gridToScreen(c.posGrid);
        const ImRect headerRect(p0, ImVec2(p0.x + c.size.x, p0.y + headerH));
        if (headerRect.Contains(mouse))
        {
            editingCommentId = c.id;
            // bring to front
            CommentBox tmp = c;
            comments.erase(comments.begin() + hoveredIdx);
            comments.push_back(tmp);
        }
    }

    const bool isEditingAny = (editingCommentId != -1);

    // Start drag/resize (disabled while editing)
    if (allowCommentInteraction && !isEditingAny && hoveredIdx != -1 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        auto& c = comments[hoveredIdx];
        const ImVec2 p0 = gridToScreen(c.posGrid);
        const ImRect headerRect(p0, ImVec2(p0.x + c.size.x, p0.y + headerH));
        const ImRect resizeRect(
            ImVec2(p0.x + c.size.x - handle, p0.y + c.size.y - handle),
            ImVec2(p0.x + c.size.x, p0.y + c.size.y));

        activeCommentId = c.id;
        resizingComment = resizeRect.Contains(mouse);
        if (!resizingComment && headerRect.Contains(mouse))
        {
            dragOffset = ImVec2(mouse.x - p0.x, mouse.y - p0.y);
        }
        else
        {
            dragOffset = ImVec2(0.0f, 0.0f);
        }

        // bring to front
        CommentBox tmp = c;
        comments.erase(comments.begin() + hoveredIdx);
        comments.push_back(tmp);
    }

    // While interacting with comments, explicitly consume the mouse drag so
    // ImNodes doesn't also begin a selection-box drag on the editor.
    // (Nodes don't have this effect because ImNodes consumes the drag itself.)
    // Only do this while actively dragging/resizing; otherwise we can block
    // node dragging when a node is over a comment.
    const bool interactingWithComment = (activeCommentId != -1) &&
                                        (resizingComment || (dragOffset.x != 0.0f || dragOffset.y != 0.0f));
    if (allowCommentInteraction && interactingWithComment)
    {
        // Mark the current window as owning the mouse.
        ImGui::SetActiveID(ImGui::GetID("##comment_drag"), ImGui::GetCurrentWindow());
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    // Continue drag/resize
    if (activeCommentId != -1 && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        for (auto& c : comments)
        {
            if (c.id != activeCommentId) continue;

            const ImVec2 p0 = gridToScreen(c.posGrid);

            if (resizingComment)
            {
                c.size.x = (mouse.x - p0.x);
                c.size.y = (mouse.y - p0.y);
                if (c.size.x < minSize.x) c.size.x = minSize.x;
                if (c.size.y < minSize.y) c.size.y = minSize.y;
            }
            else if (dragOffset.x != 0.0f || dragOffset.y != 0.0f)
            {
                // Drag in screen space, store in grid space
                const ImVec2 newPosScreen(mouse.x - dragOffset.x, mouse.y - dragOffset.y);
                c.posGrid = screenToGrid(newPosScreen);
            }
            break;
        }
    }
    if (activeCommentId != -1 && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        activeCommentId = -1;
        resizingComment = false;
        dragOffset = ImVec2(0.0f, 0.0f);

        // If we forced an ActiveID during comment dragging, clear it now so it
        // doesn't interfere with subsequent node interaction.
        if (ImGui::GetActiveID() == ImGui::GetID("##comment_drag"))
            ImGui::ClearActiveID();
    }

    // Once interaction ends, ActiveID will clear on mouse release naturally.

    // Draw + (optional) title edit
    for (auto& c : comments)
    {
        const ImVec2 p0 = gridToScreen(c.posGrid);
        const ImVec2 p1(p0.x + c.size.x, p0.y + c.size.y);

        dl->AddRectFilled(p0, p1, fillCol, 4.0f);
        dl->AddRectFilled(p0, ImVec2(p1.x, p0.y + headerH), headerCol, 4.0f);
        dl->AddRect(p0, p1, borderCol, 4.0f, 0, 2.0f);

        // Resize handle
        dl->AddRectFilled(ImVec2(p1.x - handle, p1.y - handle), p1, borderCol, 2.0f);

        // Title (read-only draw), edit only on header double-click
        const ImVec2 titlePos(p0.x + 6.0f, p0.y + 3.0f);
        if (editingCommentId == c.id)
        {
            ImGui::SetCursorScreenPos(titlePos);
            ImGui::PushID(c.id);
            ImGui::SetNextItemWidth(c.size.x - 12.0f);
            // Enter edit: focus the input.
            if (ImGui::GetActiveID() == 0)
                ImGui::SetKeyboardFocusHere();
            ImGui::InputText("##title", c.text, CommentBox::kMaxText, ImGuiInputTextFlags_AutoSelectAll);
            ImGui::PopID();
        }
        else
        {
            dl->AddText(titlePos, IM_COL32(0, 0, 0, 255), c.text);
        }
    }
}

void NodeGraph::drawNodes()
{
    for (auto& node : nodes)
    {
        // Only set the node position once when first created
        if (!node.positionSet)
        {
            // Place node in screen space at the original click position.
            // This is robust and avoids the need to convert to grid space.
            ImNodes::SetNodeScreenSpacePos(node.id, spawnPosScreen);
            node.positionSet = true;
        }

        ImNodes::BeginNode(node.id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted(node.name.c_str());
        ImNodes::EndNodeTitleBar();

        // Add input attribute
        ImNodes::BeginInputAttribute(node.id * 1000 + 1);
        ImGui::Text("Input");
        ImNodes::EndInputAttribute();

        // Add some spacing/content
        ImGui::Spacing();

        // Add output attribute
        ImNodes::BeginOutputAttribute(node.id * 1000 + 2);
        ImGui::Indent(40.f);
        ImGui::Text("Output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }
}

void NodeGraph::handleContextMenu()
{
    // Right-click on empty canvas opens popup.
    // Left-click on empty canvas closes popup.
    const bool editorHovered = ImNodes::IsEditorHovered();
    int hoveredNode = -1;
    int hoveredLink = -1;
    (void)ImNodes::IsNodeHovered(&hoveredNode);
    (void)ImNodes::IsLinkHovered(&hoveredLink);
    const bool hoveringNodeOrLink = (hoveredNode != -1) || (hoveredLink != -1);

    // Open on right click (only when not clicking on a node/link)
    if (editorHovered && !hoveringNodeOrLink && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        showContextMenu = true;
        // Capture spawn position in screen space.
        spawnPosScreen = ImGui::GetMousePos();
        // Keep these for compatibility with existing addNode signature.
        contextMenuX = spawnPosScreen.x;
        contextMenuY = spawnPosScreen.y;
        ImGui::OpenPopup("NodeContextMenu");
    }

    if (ImGui::BeginPopup("NodeContextMenu"))
    {
        // Dismiss popup if user left-clicks on empty canvas
        if (editorHovered && !hoveringNodeOrLink && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            ImGui::CloseCurrentPopup();
            showContextMenu = false;
            ImGui::EndPopup();
            return;
        }

        ImGui::Text("Add Node");
        ImGui::Separator();

        if (ImGui::MenuItem("Add Basic Node"))
        {
            addNode("Node " + std::to_string(nextNodeId), contextMenuX, contextMenuY);
            showContextMenu = false;
        }

        if (ImGui::MenuItem("Add Transform Node"))
        {
            addNode("Transform " + std::to_string(nextNodeId), contextMenuX, contextMenuY);
            showContextMenu = false;
        }

        if (ImGui::MenuItem("Add Process Node"))
        {
            addNode("Process " + std::to_string(nextNodeId), contextMenuX, contextMenuY);
            showContextMenu = false;
        }

        if (ImGui::MenuItem("Add Comment"))
        {
            // Comments are stored in grid-space; convert the captured mouse
            // screen-space position to grid-space using the same cached origin
            // used while rendering.
            const EditorSpaceCache& cache = g_editorSpaceCache;
            const ImVec2 spawnGrid = cache.valid ? ScreenToGrid(cache, spawnPosScreen)
                                                 : ImVec2(0.0f, 0.0f);
            addComment(spawnGrid);
            showContextMenu = false;
        }

        ImGui::EndPopup();
    }
    else
    {
        showContextMenu = false;
    }
}

void NodeGraph::drawUI()
{
    ImGui::Begin("node editor");

    ImNodes::BeginNodeEditor();

    // Cache a stable mapping between grid-space and screen-space for this frame.
    // ImNodes panning is an offset applied in screen-space; the screen-space
    // position corresponding to grid (0,0) is (editorTopLeftScreen + panning).
    // NOTE: GetCursorScreenPos() at this point is stable; later calls (inside
    // popups/widgets) are not.
    g_editorSpaceCache.panning = ImNodes::EditorContextGetPanning();
    const ImVec2 editorTopLeftScreen = ImGui::GetCursorScreenPos();
    g_editorSpaceCache.originScreen = {
        editorTopLeftScreen.x + g_editorSpaceCache.panning.x,
        editorTopLeftScreen.y + g_editorSpaceCache.panning.y
    };
    g_editorSpaceCache.valid = true;


    drawComments();
    drawNodes();
    handleContextMenu();

    ImNodes::EndNodeEditor();

    ImGui::End();
}








