//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODEGRAPH_COMMENTSVIEW_HPP
#define GLITTER_NODEGRAPH_COMMENTSVIEW_HPP

#include <vector>
#include <imgui.h>
#include <imnodes.h>
#include "imgui_internal.h"

#include "../Components/CommentBox.hpp"
#include "../NodeGraphEditorSpace.hpp"
#include "INodeGraphView.hpp"
#include "../NodeGraphRenderContext.hpp"

class NodeGraphCommentsView final : public INodeGraphView
{
public:
    explicit NodeGraphCommentsView(std::vector<CommentBox>& comments)
        : m_comments(comments)
    {
    }

    [[nodiscard]] NodeGraphLayer layer() const override { return NodeGraphLayer::Background; }

    // Model mutation API (kept local to this view so other future views can
    // own their own ID generation + creation logic).
    int nextCommentId = 0;

    void addComment(std::vector<CommentBox>& comments, const ImVec2& posGrid)
    {
      CommentBox c;
      c.id = nextCommentId++;
      c.posGrid = posGrid;
      comments.push_back(c);
    }

    // UI state (persistent selection vs transient interaction)
    // - selectedCommentId: persistently selected comment (e.g., for delete/hotkeys)
	  // - draggingCommentId: the comment currently capturing the mouse for drag/resize
    int selectedCommentId = -1;
	  int draggingCommentId = -1;
    bool resizingComment = false;
    ImVec2 dragOffset{0.0f, 0.0f};
    int editingCommentId = -1;

    void draw(NodeGraphRenderContext& ctx) override
    {
        const NodeGraphEditorSpace& usedCache = ctx.editorSpace.valid ? ctx.editorSpace : fallbackCache();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        const ImVec2 mouse = ctx.mouseScreen;
        const bool editorHovered = ctx.editorHovered;
        const bool leftClicked = ctx.leftClicked;

        const bool allowCommentInteraction = ctx.interaction.canInteract(NodeGraphInteractionOwner::Comments);

        const int hoveredIdx = (editorHovered && allowCommentInteraction)
          ? findHoveredCommentIdx(usedCache, mouse)
          : -1;

        handleEditingCancelOnOutsideClick(usedCache, mouse, ctx.leftClicked);

        if (editorHovered && leftClicked && ctx.interaction.isOwnedByOtherThan(NodeGraphInteractionOwner::Comments))
          selectedCommentId = -1;

        handleSelectionAndZOrder(editorHovered, allowCommentInteraction, hoveredIdx, leftClicked);
        handleEnterEditMode(usedCache, mouse, allowCommentInteraction, hoveredIdx, ctx.leftDoubleClicked);
        handleBeginInteraction(ctx, usedCache, mouse, allowCommentInteraction, hoveredIdx, leftClicked);
        updateActiveInteraction(ctx, usedCache, mouse, ctx.leftDown, ctx.leftReleased);

        renderAllComments(usedCache, *dl);
    }

private:
  std::vector<CommentBox>& m_comments;

  static constexpr float kHeaderH = 22.0f;
  static constexpr float kResizeHandle = 12.0f;
  static const ImVec2& minSize()
  {
    static const ImVec2 s(120.0f, 60.0f);
    return s;
  }

  [[nodiscard]] int findHoveredCommentIdx(const NodeGraphEditorSpace& cache, const ImVec2& mouse) const
  {
    for (int i = (int)m_comments.size() - 1; i >= 0; --i)
    {
      const CommentBox& c = m_comments[i];
      const ImVec2 p0 = NodeGraphGridToScreen(cache, c.posGrid);
      const ImRect r(p0, ImVec2(p0.x + c.size.x, p0.y + c.size.y));
      if (r.Contains(mouse))
        return i;
    }
    return -1;
  }

  void bringToFront(int idx)
  {
    if (idx < 0 || idx >= (int)m_comments.size())
      return;
    CommentBox tmp = m_comments[idx];
    m_comments.erase(m_comments.begin() + idx);
    m_comments.push_back(tmp);
  }

  void handleEditingCancelOnOutsideClick(const NodeGraphEditorSpace& cache, const ImVec2& mouse, bool leftClicked)
  {
    if (editingCommentId == -1 || !leftClicked)
      return;

    bool clickedInsideHeader = false;
    for (const auto& c : m_comments)
    {
      if (c.id != editingCommentId)
        continue;
      const ImVec2 p0 = NodeGraphGridToScreen(cache, c.posGrid);
      const ImRect headerRect(p0, ImVec2(p0.x + c.size.x, p0.y + kHeaderH));
      clickedInsideHeader = headerRect.Contains(mouse);
      break;
    }

    if (!clickedInsideHeader)
    {
      editingCommentId = -1;
      ImGui::ClearActiveID();
    }
  }

  void handleSelectionAndZOrder(bool editorHovered, bool allowCommentInteraction, int hoveredIdx, bool leftClicked)
  {
    if (!(editorHovered && leftClicked))
      return;

    if (allowCommentInteraction && hoveredIdx != -1)
    {
      selectedCommentId = m_comments[hoveredIdx].id;
      // Only update Z-order on a simple click. If we are actively dragging/resizing
      // we keep order stable to avoid breaking capture.
      if (draggingCommentId == -1)
        bringToFront(hoveredIdx);
    }
    else
    {
      // Click on empty editor clears selection.
      selectedCommentId = -1;
    }
  }

  void handleEnterEditMode(const NodeGraphEditorSpace& cache, const ImVec2& mouse, bool allowCommentInteraction, int hoveredIdx, bool leftDoubleClicked)
  {
    if (!(allowCommentInteraction && hoveredIdx != -1 && leftDoubleClicked))
      return;

    const CommentBox& c = m_comments[hoveredIdx];
    const ImVec2 p0 = NodeGraphGridToScreen(cache, c.posGrid);
    const ImRect headerRect(p0, ImVec2(p0.x + c.size.x, p0.y + kHeaderH));
    if (!headerRect.Contains(mouse))
      return;

    editingCommentId = c.id;
    bringToFront(hoveredIdx);
  }

  void handleBeginInteraction(NodeGraphRenderContext& ctx, const NodeGraphEditorSpace& cache, const ImVec2& mouse, bool allowCommentInteraction, int hoveredIdx, bool leftClicked)
  {
    if (!(allowCommentInteraction && leftClicked && hoveredIdx != -1))
      return;
    // Don't start dragging/resizing while editing text.
    if (editingCommentId != -1)
      return;

    CommentBox& c = m_comments[hoveredIdx];
    const ImVec2 p0 = NodeGraphGridToScreen(cache, c.posGrid);
    const ImRect headerRect(p0, ImVec2(p0.x + c.size.x, p0.y + kHeaderH));
    const ImRect resizeRect(
      ImVec2(p0.x + c.size.x - kResizeHandle, p0.y + c.size.y - kResizeHandle),
      ImVec2(p0.x + c.size.x, p0.y + c.size.y));

  // Only start interaction if the click is on the header (drag) or the resize handle.
  const bool wantsResize = resizeRect.Contains(mouse);
  const bool wantsDrag = headerRect.Contains(mouse);
  if (!wantsResize && !wantsDrag)
    return;

  // Attempt to claim exclusive interaction BEFORE we start the drag.
  if (!ctx.interaction.tryClaim(NodeGraphInteractionOwner::Comments, 20))
    return;

  // Bring to front first (changes vector order), then capture the id of the moved element.
  bringToFront(hoveredIdx);
  draggingCommentId = m_comments.empty() ? -1 : m_comments.back().id;
  resizingComment = wantsResize;
  if (!resizingComment)
    dragOffset = ImVec2(mouse.x - p0.x, mouse.y - p0.y);
  else
    dragOffset = ImVec2(0.0f, 0.0f);
  }

	CommentBox* findById(int id)
	{
		for (auto& c : m_comments)
			if (c.id == id)
				return &c;
		return nullptr;
	}

  void updateActiveInteraction(NodeGraphRenderContext& ctx, const NodeGraphEditorSpace& cache, const ImVec2& mouse, bool leftDown, bool leftReleased)
  {
    const bool hasMouseCapture = (draggingCommentId != -1) &&
      (resizingComment || (dragOffset.x != 0.0f || dragOffset.y != 0.0f));

    // Keep cursor stable while dragging/resizing (prevents flicker).
    if (hasMouseCapture)
    {
      ImGui::SetActiveID(ImGui::GetID("##comment_drag"), ImGui::GetCurrentWindow());
      ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    if (draggingCommentId != -1 && leftDown)
    {
      if (CommentBox* c = findById(draggingCommentId))
      {
            const ImVec2 p0 = NodeGraphGridToScreen(cache, c->posGrid);

        if (resizingComment)
        {
          c->size.x = (mouse.x - p0.x);
          c->size.y = (mouse.y - p0.y);
          if (c->size.x < minSize().x) c->size.x = minSize().x;
          if (c->size.y < minSize().y) c->size.y = minSize().y;
        }
        else if (dragOffset.x != 0.0f || dragOffset.y != 0.0f)
        {
          const ImVec2 newPosScreen(mouse.x - dragOffset.x, mouse.y - dragOffset.y);
          c->posGrid = NodeGraphScreenToGrid(cache, newPosScreen);
        }
      }
    }
    if (draggingCommentId != -1 && leftReleased)
    {
      draggingCommentId = -1;
      resizingComment = false;
      dragOffset = ImVec2(0.0f, 0.0f);
      ctx.interaction.release(NodeGraphInteractionOwner::Comments);

      if (ImGui::GetActiveID() == ImGui::GetID("##comment_drag"))
        ImGui::ClearActiveID();
    }
  }

  void renderAllComments(const NodeGraphEditorSpace& cache, ImDrawList& dl)
  {
    for (auto& c : m_comments)
      renderOneComment(cache, dl, c);
  }

  void renderOneComment(const NodeGraphEditorSpace& cache, ImDrawList& dl, CommentBox& c) const
  {
    const ImU32 fillCol = IM_COL32(255, 255, 0, 40);
    const ImU32 borderCol = IM_COL32(255, 255, 0, 140);
    const ImU32 headerCol = IM_COL32(255, 255, 0, 80);
    const ImU32 selBorderCol = IM_COL32(255, 255, 0, 220);

    const ImVec2 p0 = NodeGraphGridToScreen(cache, c.posGrid);
    const ImVec2 p1(p0.x + c.size.x, p0.y + c.size.y);

    dl.AddRectFilled(p0, p1, fillCol, 4.0f);
    dl.AddRectFilled(p0, ImVec2(p1.x, p0.y + kHeaderH), headerCol, 4.0f);
    if (selectedCommentId == c.id)
      dl.AddRect(p0, p1, selBorderCol, 4.0f, 0, 3.0f);
    else
      dl.AddRect(p0, p1, borderCol, 4.0f, 0, 2.0f);

    dl.AddRectFilled(ImVec2(p1.x - kResizeHandle, p1.y - kResizeHandle), p1, borderCol, 2.0f);

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
      dl.AddText(titlePos, IM_COL32(0, 0, 0, 255), c.text);
    }
  }

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



