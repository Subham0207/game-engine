//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODEGRAPH_COMMENTSVIEW_HPP
#define GLITTER_NODEGRAPH_COMMENTSVIEW_HPP

#include <vector>
#include <imgui.h>
#include <imnodes.h>
#include "imgui_internal.h"

#include "CommentBox.hpp"
#include "NodeGraphEditorSpace.hpp"
#include "NodeGraphNode.hpp"
#include "INodeGraphView.hpp"
#include "NodeGraphRenderContext.hpp"

class NodeGraphCommentsView final : public INodeGraphView
{
public:
    explicit NodeGraphCommentsView(std::vector<CommentBox>& comments)
        : m_comments(comments)
    {
    }

    [[nodiscard]] NodeGraphLayer layer() const override { return NodeGraphLayer::Background; }

    // Model mutation API
    int nextCommentId = 0;

    void addComment(std::vector<CommentBox>& comments, const ImVec2& posGrid)
    {
        CommentBox c;
        c.id = nextCommentId++;
        c.posGrid = posGrid;
        comments.push_back(c);
    }

    // State previously stored in NodeGraph
    int activeCommentId = -1;
    bool resizingComment = false;
    ImVec2 dragOffset{0.0f, 0.0f};
    int editingCommentId = -1;
    int selectedCommentId = -1;

    void draw(NodeGraphRenderContext& ctx) override
    {
        const NodeGraphEditorSpace& cache = ctx.editorSpace;
        const std::vector<NodeGraphNode>& nodes = ctx.nodes;

        ImDrawList* dl = ImGui::GetWindowDrawList();

        const NodeGraphEditorSpace& usedCache = cache.valid ? cache : fallbackCache();

        auto gridToScreen = [&](const ImVec2& g) -> ImVec2 { return NodeGraphGridToScreen(usedCache, g); };
        auto screenToGrid = [&](const ImVec2& s) -> ImVec2 { return NodeGraphScreenToGrid(usedCache, s); };

        const ImU32 fillCol = IM_COL32(255, 255, 0, 40);
        const ImU32 borderCol = IM_COL32(255, 255, 0, 140);
        const ImU32 headerCol = IM_COL32(255, 255, 0, 80);
        const ImU32 selBorderCol = IM_COL32(255, 255, 0, 220);

        const float headerH = 22.0f;
        const float handle = 12.0f;
        const ImVec2 minSize(120.0f, 60.0f);

        int hoveredIdx = -1;

        int hoveredNode = -1;
        int hoveredLink = -1;
        int hoveredPin = -1;
        (void)ImNodes::IsNodeHovered(&hoveredNode);
        (void)ImNodes::IsLinkHovered(&hoveredLink);
        (void)ImNodes::IsPinHovered(&hoveredPin);

        int activeAttribute = -1;
        const bool anyAttributeActive = ImNodes::IsAnyAttributeActive(&activeAttribute);

        const bool hoveringImNodesElement = (hoveredNode != -1) || (hoveredLink != -1) || (hoveredPin != -1);
        const bool leftClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

        const bool editorHovered = ImNodes::IsEditorHovered();

        bool mouseOverAnyNodeRect = false;
        if (editorHovered)
        {
            const ImVec2 m = ImGui::GetMousePos();
            for (const auto& n : nodes)
            {
                // ImNodes node rect queries are only valid after the node has been
                // submitted at least once. If we query too early, some builds can AV.
                if (!n.positionSet())
                    continue;

                const ImVec2 nd = ImNodes::GetNodeDimensions(n.id());
                if (nd.x <= 0.0f || nd.y <= 0.0f)
                    continue;

                const ImVec2 np = ImNodes::GetNodeScreenSpacePos(n.id());
                const ImRect nr(np, ImVec2(np.x + nd.x, np.y + nd.y));
                if (nr.Contains(m))
                {
                    mouseOverAnyNodeRect = true;
                    break;
                }
            }
        }

        const bool clickOnImNodesElement = leftClicked && editorHovered &&
                                          (hoveringImNodesElement || anyAttributeActive || mouseOverAnyNodeRect);

        const bool imguiItemHoveredOrActive = ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive();
        const bool allowCommentInteraction = !(clickOnImNodesElement || imguiItemHoveredOrActive);

        const ImVec2 mouse = ImGui::GetMousePos();
        if (ImNodes::IsEditorHovered() && allowCommentInteraction)
        {
            for (int i = (int)m_comments.size() - 1; i >= 0; --i)
            {
                const CommentBox& c = m_comments[i];
                const ImVec2 p0 = gridToScreen(c.posGrid);
                const ImRect r(p0, ImVec2(p0.x + c.size.x, p0.y + c.size.y));
                if (r.Contains(mouse))
                {
                    hoveredIdx = i;
                    break;
                }
            }
        }

        if (editingCommentId != -1 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            bool clickedInsideEditing = false;
            for (const auto& c : m_comments)
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

        if (editorHovered && clickOnImNodesElement)
            selectedCommentId = -1;

        if (allowCommentInteraction && hoveredIdx != -1 && leftClicked)
        {
            const auto& c = m_comments[hoveredIdx];
            selectedCommentId = c.id;
            CommentBox tmp = c;
            m_comments.erase(m_comments.begin() + hoveredIdx);
            m_comments.push_back(tmp);
            hoveredIdx = (int)m_comments.size() - 1;
        }
        else if (editorHovered && leftClicked && hoveredIdx == -1 && allowCommentInteraction)
        {
            selectedCommentId = -1;
        }

        if (allowCommentInteraction && hoveredIdx != -1 && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            const auto& c = m_comments[hoveredIdx];
            const ImVec2 p0 = gridToScreen(c.posGrid);
            const ImRect headerRect(p0, ImVec2(p0.x + c.size.x, p0.y + headerH));
            if (headerRect.Contains(mouse))
            {
                editingCommentId = c.id;
                CommentBox tmp = c;
                m_comments.erase(m_comments.begin() + hoveredIdx);
                m_comments.push_back(tmp);
            }
        }

        const bool isEditingAny = (editingCommentId != -1);

        if (allowCommentInteraction && !isEditingAny && hoveredIdx != -1 && leftClicked)
        {
            auto& c = m_comments[hoveredIdx];
            const ImVec2 p0 = gridToScreen(c.posGrid);
            const ImRect headerRect(p0, ImVec2(p0.x + c.size.x, p0.y + headerH));
            const ImRect resizeRect(
                ImVec2(p0.x + c.size.x - handle, p0.y + c.size.y - handle),
                ImVec2(p0.x + c.size.x, p0.y + c.size.y));

            activeCommentId = c.id;
            resizingComment = resizeRect.Contains(mouse);
            if (!resizingComment && headerRect.Contains(mouse))
                dragOffset = ImVec2(mouse.x - p0.x, mouse.y - p0.y);
            else
                dragOffset = ImVec2(0.0f, 0.0f);

            CommentBox tmp = c;
            m_comments.erase(m_comments.begin() + hoveredIdx);
            m_comments.push_back(tmp);
        }

        const bool interactingWithComment = (activeCommentId != -1) &&
                                            (resizingComment || (dragOffset.x != 0.0f || dragOffset.y != 0.0f));

        // While a comment is actively being dragged/resized, keep the cursor stable.
        // If we gate SetMouseCursor on allowCommentInteraction, it can flicker because
        // allowCommentInteraction depends on hover tests that change as things move.
        if (interactingWithComment)
        {
            ImGui::SetActiveID(ImGui::GetID("##comment_drag"), ImGui::GetCurrentWindow());
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        if (activeCommentId != -1 && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            for (auto& c : m_comments)
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

            if (ImGui::GetActiveID() == ImGui::GetID("##comment_drag"))
                ImGui::ClearActiveID();
        }

        for (auto& c : m_comments)
        {
            const ImVec2 p0 = gridToScreen(c.posGrid);
            const ImVec2 p1(p0.x + c.size.x, p0.y + c.size.y);

            dl->AddRectFilled(p0, p1, fillCol, 4.0f);
            dl->AddRectFilled(p0, ImVec2(p1.x, p0.y + headerH), headerCol, 4.0f);
            if (selectedCommentId == c.id)
                dl->AddRect(p0, p1, selBorderCol, 4.0f, 0, 3.0f);
            else
                dl->AddRect(p0, p1, borderCol, 4.0f, 0, 2.0f);

            dl->AddRectFilled(ImVec2(p1.x - handle, p1.y - handle), p1, borderCol, 2.0f);

            const ImVec2 titlePos(p0.x + 6.0f, p0.y + 3.0f);
            if (editingCommentId == c.id)
            {
                ImGui::SetCursorScreenPos(titlePos);
                ImGui::PushID(c.id);
                ImGui::SetNextItemWidth(c.size.x - 12.0f);
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

private:
    std::vector<CommentBox>& m_comments;

    static const NodeGraphEditorSpace& fallbackCache()
    {
        static NodeGraphEditorSpace fallback;
        fallback.panning = ImNodes::EditorContextGetPanning();
        fallback.originScreen = ImGui::GetCursorScreenPos();
        fallback.originScreen.x += fallback.panning.x;
        fallback.originScreen.y += fallback.panning.y;
        fallback.valid = true;
        return fallback;
    }
};

#endif //GLITTER_NODEGRAPH_COMMENTSVIEW_HPP



