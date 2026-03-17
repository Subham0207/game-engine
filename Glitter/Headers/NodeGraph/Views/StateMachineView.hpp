//
// Created by subha on 17-03-2026.
//

#ifndef GLITTER_NODEGRAPH_STATEMACHINEVIEW_HPP
#define GLITTER_NODEGRAPH_STATEMACHINEVIEW_HPP

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

#include <imgui.h>
#include <imnodes.h>

#include "imgui_internal.h"

#include "INodeGraphView.hpp"
#include "../NodeGraphRenderContext.hpp"
#include "../Components/StateMachineNode.hpp"
#include "../Components/StateMachineTransition.hpp"

// View: draws and edits a prototype state-machine graph.
// - State nodes are ImNodes nodes (we rely on ImNodes for node selection + movement).
// - Links are fully custom rendered & interactive, with endpoints that can attach
//   anywhere on the node body (any direction, multiple links can share the same point).
class StateMachineView final : public INodeGraphView
{
public:
    [[nodiscard]] NodeGraphLayer layer() const override { return NodeGraphLayer::Content; }

    // Draw after basic node view by default.
    // (Smaller number draws earlier within a layer.)
    [[nodiscard]] int priority() const override { return 5; }

    int nextStateId = 0;
    int nextTransitionId = 0;

    void addState(std::vector<StateMachineNode>& nodes, const std::string& baseName, const ImVec2& spawnPosScreen)
    {
        StateMachineNode n;
        n.id = nextStateId++;
        n.name = baseName + " " + std::to_string(n.id);
        n.spawnPosScreen = spawnPosScreen;
        n.hasSpawn = true;
        nodes.emplace_back(std::move(n));
    }

    static void setActive(std::vector<StateMachineNode>& nodes, int nodeId)
    {
        for (auto& n : nodes)
            n.active = (n.id == nodeId);
    }

    void draw(NodeGraphRenderContext& ctx) override
    {
        // We draw custom links (and handle link interactions) inside this view for now.
        // If you later want links shared across multiple node types, split this into a separate Overlay view.

        if (ctx.editorHovered)
        {
            int hoveredNode = -1;
            int hoveredLink = -1;
            int hoveredPin = -1;
            (void)ImNodes::IsNodeHovered(&hoveredNode);
            (void)ImNodes::IsLinkHovered(&hoveredLink);
            (void)ImNodes::IsPinHovered(&hoveredPin);

            int activeAttribute = -1;
            const bool anyAttributeActive = ImNodes::IsAnyAttributeActive(&activeAttribute);
            const bool hoveringImNodesElement = (hoveredNode != -1) || (hoveredLink != -1) || (hoveredPin != -1);
            if (hoveringImNodesElement || anyAttributeActive)
                ctx.interaction.tryClaim(NodeGraphInteractionOwner::ImNodes, 10);
        }

        auto& nodes = ctx.stateNodes;
        auto& links = ctx.stateTransitions;

        if (!ctx.editorSpace.valid)
            return;

        // Keep per-frame rect cache fresh.
        m_nodeRects.clear();

        // Provide a default active state.
        if (!nodes.empty() && findActiveNodeId(nodes) == -1)
            setActive(nodes, nodes.front().id);

        // --- Render nodes (ImNodes only)
        for (auto& n : nodes)
        {
            if (!n.posSet)
            {
                const ImVec2 initial = n.hasSpawn ? n.spawnPosScreen : ImGui::GetMousePos();
                ImNodes::SetNodeScreenSpacePos(n.id, initial);
                n.posSet = true;
            }

            const bool isActive = n.active;
            if (isActive)
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(80, 160, 80, 255));

            ImNodes::BeginNode(n.id);
            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted(n.name.c_str());
            ImNodes::EndNodeTitleBar();

            // Context-ish controls inside node.
            ImGui::Spacing();
            if (ImGui::SmallButton((std::string("Make Active##") + std::to_string(n.id)).c_str()))
                setActive(nodes, n.id);

            ImNodes::EndNode();

            // Cache node rect for custom link hit-testing/rendering.
            // ImNodes gives us position + dimensions in screen-space.
            const ImVec2 pos = ImNodes::GetNodeScreenSpacePos(n.id);
            const ImVec2 dims = ImNodes::GetNodeDimensions(n.id);
            m_nodeRects[n.id] = ImRect(pos, ImVec2(pos.x + dims.x, pos.y + dims.y));

            if (isActive)
                ImNodes::PopColorStyle();
        }

        // --- Custom links: render + hit-test + interaction
        drawAndInteractLinks(ctx, nodes, links);

        // --- Prototype simulation: if the active state has an outgoing transition whose condition == "true",
        // switch to its target.
        // This is intentionally minimal; later you can evaluate real conditions.
        if (!ctx.leftDown && ImGui::IsKeyPressed(ImGuiKey_Space))
        {
            const int activeId = findActiveNodeId(nodes);
            if (activeId != -1)
            {
                for (const auto& t : links)
                {
                    if (t.fromNodeId == activeId && (t.condition == "true" || t.condition == "True"))
                    {
                        setActive(nodes, t.toNodeId);
                        break;
                    }
                }
            }
        }
    }

private:
    struct DragState
    {
        bool active = false;
        int transitionId = -1;
        bool draggingFrom = true; // true=dragging from-end, false=to-end
        ImVec2 mouseScreen{0.0f, 0.0f};
    };

  struct PortHover
  {
    bool valid = false;
    int nodeId = -1;
    StateMachinePortSide side = StateMachinePortSide::West;
    ImVec2 pointScreen{0.0f, 0.0f};
    ImVec2 offsetGrid{0.0f, 0.0f};
  };

    static ImVec2 endpointToScreen(const NodeGraphEditorSpace& space, const ImRect& nodeRectScreen, const ImVec2& offsetGrid)
    {
        const ImVec2 nodeMinGrid = NodeGraphScreenToGrid(space, nodeRectScreen.Min);
        const ImVec2 pGrid(nodeMinGrid.x + offsetGrid.x, nodeMinGrid.y + offsetGrid.y);
        return NodeGraphGridToScreen(space, pGrid);
    }

    static ImVec2 mouseOffsetGridOnNode(const NodeGraphEditorSpace& space, const ImRect& nodeRectScreen, const ImVec2& mouseScreen)
    {
        const ImVec2 mouseGrid = NodeGraphScreenToGrid(space, mouseScreen);
        const ImVec2 nodeMinGrid = NodeGraphScreenToGrid(space, nodeRectScreen.Min);
        return ImVec2(mouseGrid.x - nodeMinGrid.x, mouseGrid.y - nodeMinGrid.y);
    }

    static StateMachineTransition* findTransition(std::vector<StateMachineTransition>& links, int id)
    {
        for (auto& t : links)
        {
            if (t.id == id)
                return &t;
        }
        return nullptr;
    }

    static void eraseTransition(std::vector<StateMachineTransition>& links, int id)
    {
        links.erase(std::remove_if(links.begin(), links.end(), [&](const StateMachineTransition& t) { return t.id == id; }), links.end());
    }

    static int findActiveNodeId(const std::vector<StateMachineNode>& nodes)
    {
        for (const auto& n : nodes)
        {
            if (n.active)
                return n.id;
        }
        return -1;
    }

    // Custom link helpers
    static constexpr float kEndpointRadius = 6.0f;
    static constexpr float kEndpointHitRadius = 10.0f;
    static constexpr float kBezierTangent = 80.0f;
    static constexpr float kLinkHitDist = 8.0f;

    // We reuse StateMachineTransition's fromSide/toSide fields as a compact storage for
    // the nearest-edge direction of the endpoint (helps bezier direction). The actual
    // endpoint position is stored persistently in StateMachineTransition::fromOffsetGrid/toOffsetGrid.

    // Per-frame node rect cache in screen-space.
    std::unordered_map<int, ImRect> m_nodeRects;

    DragState m_drag;
    int m_selectedTransitionId = -1;
	bool m_pendingLinkActive = false;
	PortHover m_pendingStart;

    static StateMachinePortSide computeNearestSide(const ImRect& r, const ImVec2& p)
    {
        const float dl = fabsf(p.x - r.Min.x);
        const float dr = fabsf(r.Max.x - p.x);
        const float dt = fabsf(p.y - r.Min.y);
        const float db = fabsf(r.Max.y - p.y);
        float best = dl;
        StateMachinePortSide side = StateMachinePortSide::West;
        if (dr < best) { best = dr; side = StateMachinePortSide::East; }
        if (dt < best) { best = dt; side = StateMachinePortSide::North; }
        if (db < best) { best = db; side = StateMachinePortSide::South; }
        return side;
    }

    static ImVec2 sideDir(StateMachinePortSide s)
    {
        switch (s)
        {
        case StateMachinePortSide::North: return ImVec2(0.0f, -1.0f);
        case StateMachinePortSide::East: return ImVec2(1.0f, 0.0f);
        case StateMachinePortSide::South: return ImVec2(0.0f, 1.0f);
        case StateMachinePortSide::West: return ImVec2(-1.0f, 0.0f);
        }
        return ImVec2(1.0f, 0.0f);
    }

    static float distPointToSegment(const ImVec2& p, const ImVec2& a, const ImVec2& b)
    {
        const ImVec2 ab(b.x - a.x, b.y - a.y);
        const ImVec2 ap(p.x - a.x, p.y - a.y);
        const float abLen2 = ab.x * ab.x + ab.y * ab.y;
        if (abLen2 <= 1e-4f)
        {
            const float dx = p.x - a.x;
            const float dy = p.y - a.y;
            return sqrtf(dx * dx + dy * dy);
        }
        float t = (ap.x * ab.x + ap.y * ab.y) / abLen2;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        const ImVec2 c(a.x + ab.x * t, a.y + ab.y * t);
        const float dx = p.x - c.x;
        const float dy = p.y - c.y;
        return sqrtf(dx * dx + dy * dy);
    }

    float hitTestBezier(const ImVec2& mouse, const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3) const
    {
        // Approximate with polyline segments.
        constexpr int kSegs = 16;
        ImVec2 prev = p0;
        float best = 1e9f;
        for (int i = 1; i <= kSegs; ++i)
        {
            const float t = (float)i / (float)kSegs;
            const float u = 1.0f - t;
            const ImVec2 pt(
                u * u * u * p0.x + 3 * u * u * t * p1.x + 3 * u * t * t * p2.x + t * t * t * p3.x,
                u * u * u * p0.y + 3 * u * u * t * p1.y + 3 * u * t * t * p2.y + t * t * t * p3.y);
            best = std::min(best, distPointToSegment(mouse, prev, pt));
            prev = pt;
        }
        return best;
    }

    bool getNodeAt(const ImVec2& mouseScreen, int& outNodeId, ImRect& outRect) const
    {
        for (const auto& kv : m_nodeRects)
        {
            if (kv.second.Contains(mouseScreen))
            {
                outNodeId = kv.first;
                outRect = kv.second;
                return true;
            }
        }
        return false;
    }

    bool endpointScreenForTransition(const NodeGraphEditorSpace& space, const StateMachineTransition& t, bool from, ImVec2& outP) const
    {
        const int nodeId = from ? t.fromNodeId : t.toNodeId;
        auto nr = m_nodeRects.find(nodeId);
        if (nr == m_nodeRects.end())
            return false;
        const ImVec2 offset = from ? t.fromOffsetGrid : t.toOffsetGrid;
        outP = endpointToScreen(space, nr->second, offset);
        return true;
    }

    void ensureEndpointsInitialized(const NodeGraphEditorSpace& space, std::vector<StateMachineTransition>& links)
    {
        for (auto& t : links)
        {
            // If endpoints missing, initialize them to a reasonable default on the border
            // (center-left/right).
            if (t.fromOffsetGrid.x == 0.0f && t.fromOffsetGrid.y == 0.0f)
            {
                auto it = m_nodeRects.find(t.fromNodeId);
                if (it != m_nodeRects.end())
                {
                    const ImRect r = it->second;
                    const ImVec2 p(r.Max.x, (r.Min.y + r.Max.y) * 0.5f);
                    t.fromSide = computeNearestSide(r, p);
                    t.fromOffsetGrid = mouseOffsetGridOnNode(space, r, p);
                }
            }
            if (t.toOffsetGrid.x == 0.0f && t.toOffsetGrid.y == 0.0f)
            {
                auto it = m_nodeRects.find(t.toNodeId);
                if (it != m_nodeRects.end())
                {
                    const ImRect r = it->second;
                    const ImVec2 p(r.Min.x, (r.Min.y + r.Max.y) * 0.5f);
                    t.toSide = computeNearestSide(r, p);
                    t.toOffsetGrid = mouseOffsetGridOnNode(space, r, p);
                }
            }
        }
    }

    void drawAndInteractLinks(NodeGraphRenderContext& ctx, std::vector<StateMachineNode>& nodes, std::vector<StateMachineTransition>& links)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 mouse = ctx.mouseScreen;

        ensureEndpointsInitialized(ctx.editorSpace, links);

    // --- Port hover logic (for starting/ending links)
    PortHover hoveredPort = computeHoveredPort(ctx);
    bool portClickedThisFrame = false;
    PortHover clickedPort;
    bool startedPendingThisFrame = false;
    // If the mouse is over a port (or we are dragging a pending link), aggressively capture
    // mouse input to prevent ImNodes from starting marquee selection.
    // Note: SetNextFrameWantCaptureMouse alone is not enough for ImNodes; we need to also
    // swallow the drag by capturing while mouse is down.
    const bool wantPortInteraction = hoveredPort.valid || m_pendingLinkActive;
    if (wantPortInteraction)
    {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
      ImGui::SetNextFrameWantCaptureMouse(true);
      if (ctx.leftDown)
        ImGui::SetNextFrameWantCaptureMouse(true);

      // Create an invisible hit-box over the port region so ImGui consumes the click/drag.
      // This prevents ImNodes from starting marquee selection in the background.
      if (hoveredPort.valid)
      {
        ImGui::SetCursorScreenPos(ImVec2(hoveredPort.pointScreen.x - kEndpointHitRadius, hoveredPort.pointScreen.y - kEndpointHitRadius));
        ImGui::InvisibleButton("##SM_PortHit", ImVec2(kEndpointHitRadius * 2.0f, kEndpointHitRadius * 2.0f));
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
          portClickedThisFrame = true;
          clickedPort = hoveredPort;
        }
      }
    }
    renderPortHover(dl, hoveredPort, m_pendingLinkActive);

    // Cancel pending start if user clicks away.
    // Do not cancel on the same frame we clicked a port hitbox; that click is handled below.
    if (m_pendingLinkActive && ctx.leftClicked && !hoveredPort.valid && !portClickedThisFrame)
    {
      m_pendingLinkActive = false;
      m_pendingStart = PortHover{};
    }

    // Start a new link ONLY when clicking a hovered port.
    // We claim ownership with a higher priority than ImNodes so that even if ImNodes also
    // wants the mouse this frame, our port interaction wins.
    if (!m_pendingLinkActive && portClickedThisFrame && clickedPort.valid && ctx.interaction.tryClaim(NodeGraphInteractionOwner::Other, 1000))
    {
      m_pendingLinkActive = true;
      m_pendingStart = clickedPort;
      startedPendingThisFrame = true;
    }

    // If we have a pending start, drag a preview line.
    if (m_pendingLinkActive)
    {
      const ImU32 previewCol = IM_COL32(200, 200, 220, 200);
      const ImVec2 p0 = m_pendingStart.pointScreen;
      const ImVec2 d0 = sideDir(m_pendingStart.side);
      const ImVec2 c1(p0.x + d0.x * kBezierTangent, p0.y + d0.y * kBezierTangent);
      const ImVec2 c2(mouse.x, mouse.y);
      dl->AddBezierCubic(p0, c1, c2, mouse, previewCol, 2.0f);
    }

    // Finish: if pending and clicking a hovered port on another node, create transition.
    // Guard against the click that started the pending link; otherwise we'd start+finish immediately.
    if (m_pendingLinkActive && !startedPendingThisFrame && portClickedThisFrame && clickedPort.valid)
    {
      // If user clicked the same port again, treat it as cancel.
      if (clickedPort.nodeId == m_pendingStart.nodeId && clickedPort.side == m_pendingStart.side)
      {
        m_pendingLinkActive = false;
        m_pendingStart = PortHover{};
        ctx.interaction.release(NodeGraphInteractionOwner::Other);
      }
      else
      {
        StateMachineTransition tr;
        tr.id = nextTransitionId++;
        tr.fromNodeId = m_pendingStart.nodeId;
        tr.toNodeId = clickedPort.nodeId;
        tr.fromSide = m_pendingStart.side;
        tr.toSide = clickedPort.side;
        tr.fromOffsetGrid = m_pendingStart.offsetGrid;
        tr.toOffsetGrid = clickedPort.offsetGrid;
        tr.condition = "true";

        links.emplace_back(std::move(tr));
        m_pendingLinkActive = false;
        m_pendingStart = PortHover{};
        ctx.interaction.release(NodeGraphInteractionOwner::Other);
      }
    }

        // Hit test endpoints and links.
        int hoveredTransition = -1;
        bool hoveredFrom = false;
        float bestEndpointDist = 1e9f;
        float bestLinkDist = 1e9f;

        for (auto& t : links)
        {
            ImVec2 pFrom, pTo;
            if (!endpointScreenForTransition(ctx.editorSpace, t, true, pFrom) || !endpointScreenForTransition(ctx.editorSpace, t, false, pTo))
                continue;

            const float df = sqrtf((mouse.x - pFrom.x) * (mouse.x - pFrom.x) + (mouse.y - pFrom.y) * (mouse.y - pFrom.y));
            const float dt = sqrtf((mouse.x - pTo.x) * (mouse.x - pTo.x) + (mouse.y - pTo.y) * (mouse.y - pTo.y));
            if (df < bestEndpointDist && df <= kEndpointHitRadius)
            {
                bestEndpointDist = df;
                hoveredTransition = t.id;
                hoveredFrom = true;
            }
            if (dt < bestEndpointDist && dt <= kEndpointHitRadius)
            {
                bestEndpointDist = dt;
                hoveredTransition = t.id;
                hoveredFrom = false;
            }

            // Bezier hit test
            const ImVec2 d0 = sideDir(t.fromSide);
            const ImVec2 d1 = sideDir(t.toSide);
            const ImVec2 c1(pFrom.x + d0.x * kBezierTangent, pFrom.y + d0.y * kBezierTangent);
            const ImVec2 c2(pTo.x + d1.x * kBezierTangent, pTo.y + d1.y * kBezierTangent);
            const float dist = hitTestBezier(mouse, pFrom, c1, c2, pTo);
            if (dist < bestLinkDist)
            {
                bestLinkDist = dist;
                if (bestEndpointDist > kEndpointHitRadius)
                {
                    // only consider link hover if not on an endpoint
                    hoveredTransition = (dist <= kLinkHitDist) ? t.id : hoveredTransition;
                }
            }
        }

        // Begin interactions
        // - Click on endpoint: drag that endpoint (reconnect)
        // - Click on link: select + open popup for condition

        const bool allow = ctx.interaction.canInteract(NodeGraphInteractionOwner::Other);

        if (allow && ctx.leftClicked)
        {
            if (hoveredTransition != -1 && bestEndpointDist <= kEndpointHitRadius)
            {
                // Drag endpoint
                ctx.interaction.tryClaim(NodeGraphInteractionOwner::Other, 30);
                m_drag.active = true;
                m_drag.transitionId = hoveredTransition;
                m_drag.draggingFrom = hoveredFrom;
                m_drag.mouseScreen = mouse;
            }
            else
            {
                if (hoveredTransition != -1 && bestLinkDist <= kLinkHitDist)
                {
                    // Select link
                    m_selectedTransitionId = hoveredTransition;
                    ImGui::OpenPopup("SM_TransitionEdit");
                }
            }
        }

        // Update drag
        if (m_drag.active)
        {
            if (ctx.interaction.isOwnedBy(NodeGraphInteractionOwner::Other))
            {
                StateMachineTransition* tr = findTransition(links, m_drag.transitionId);
                if (tr)
                {
                    int nodeId = -1;
                    ImRect r;
                    if (getNodeAt(mouse, nodeId, r))
                    {
                        const StateMachinePortSide side = computeNearestSide(r, mouse);
                        if (m_drag.draggingFrom)
                        {
                            tr->fromNodeId = nodeId;
                            tr->fromSide = side;
                            tr->fromOffsetGrid = mouseOffsetGridOnNode(ctx.editorSpace, r, mouse);
                        }
                        else
                        {
                            tr->toNodeId = nodeId;
                            tr->toSide = side;
                            tr->toOffsetGrid = mouseOffsetGridOnNode(ctx.editorSpace, r, mouse);
                        }
                    }
                    else
                    {
                        // Not over any node: keep the moved end as "floating" visually by updating its offset
                        // relative to its current node (clamped inside rect if available).
                        const int currentNodeId = m_drag.draggingFrom ? tr->fromNodeId : tr->toNodeId;
                        auto nr = m_nodeRects.find(currentNodeId);
                        if (nr != m_nodeRects.end())
                        {
                            // Clamp to node bounds so it doesn't explode.
                            ImVec2 clamped = mouse;
                            clamped.x = std::max(nr->second.Min.x, std::min(nr->second.Max.x, clamped.x));
                            clamped.y = std::max(nr->second.Min.y, std::min(nr->second.Max.y, clamped.y));
                            if (m_drag.draggingFrom)
                            {
                                tr->fromOffsetGrid = mouseOffsetGridOnNode(ctx.editorSpace, nr->second, clamped);
                                tr->fromSide = computeNearestSide(nr->second, clamped);
                            }
                            else
                            {
                                tr->toOffsetGrid = mouseOffsetGridOnNode(ctx.editorSpace, nr->second, clamped);
                                tr->toSide = computeNearestSide(nr->second, clamped);
                            }
                        }
                    }
                }
            }

            if (ctx.leftReleased)
            {
                // Finalize: if both ends are on valid nodes, keep; otherwise delete.
                StateMachineTransition* tr = findTransition(links, m_drag.transitionId);
                bool ok = false;
                if (tr)
                {
                    ok = (m_nodeRects.find(tr->fromNodeId) != m_nodeRects.end()) && (m_nodeRects.find(tr->toNodeId) != m_nodeRects.end());
                    if (!ok)
                        eraseTransition(links, tr->id);
                }
                m_drag = DragState{};
                ctx.interaction.release(NodeGraphInteractionOwner::Other);
            }
        }

        // Render links + endpoints (after interaction updates)
        for (auto& t : links)
        {
            ImVec2 pFrom, pTo;
            if (!endpointScreenForTransition(ctx.editorSpace, t, true, pFrom) || !endpointScreenForTransition(ctx.editorSpace, t, false, pTo))
                continue;

            const bool selected = (t.id == m_selectedTransitionId);
            const bool hovered = (t.id == hoveredTransition);
            const ImU32 col = selected ? IM_COL32(255, 220, 120, 255) : hovered ? IM_COL32(180, 220, 255, 255) : IM_COL32(200, 200, 200, 255);
            const float thick = selected ? 3.0f : hovered ? 2.5f : 2.0f;

            const ImVec2 d0 = sideDir(t.fromSide);
            const ImVec2 d1 = sideDir(t.toSide);
            const ImVec2 c1(pFrom.x + d0.x * kBezierTangent, pFrom.y + d0.y * kBezierTangent);
            const ImVec2 c2(pTo.x + d1.x * kBezierTangent, pTo.y + d1.y * kBezierTangent);

            dl->AddBezierCubic(pFrom, c1, c2, pTo, col, thick);

            // Endpoints
            dl->AddCircleFilled(pFrom, kEndpointRadius, IM_COL32(60, 60, 60, 255));
            dl->AddCircle(pFrom, kEndpointRadius, col, 12, 2.0f);
            dl->AddCircleFilled(pTo, kEndpointRadius, IM_COL32(60, 60, 60, 255));
            dl->AddCircle(pTo, kEndpointRadius, col, 12, 2.0f);

            // Quick condition label near the middle.
            const ImVec2 mid((pFrom.x + pTo.x) * 0.5f, (pFrom.y + pTo.y) * 0.5f);
            if (!t.condition.empty())
                dl->AddText(ImVec2(mid.x + 6.0f, mid.y + 6.0f), IM_COL32(240, 240, 240, 255), t.condition.c_str());
        }

        // Condition edit popup
        if (ImGui::BeginPopup("SM_TransitionEdit"))
        {
            StateMachineTransition* tr = findTransition(links, m_selectedTransitionId);
            if (tr)
            {
                ImGui::Text("Transition %d", tr->id);

                char buf[256];
                buf[0] = '\0';
                if (!tr->condition.empty())
                    strncpy_s(buf, tr->condition.c_str(), sizeof(buf) - 1);

                if (ImGui::InputText("Condition", buf, sizeof(buf)))
                    tr->condition = buf;

                if (ImGui::Button("Delete"))
                {
                    eraseTransition(links, tr->id);
                    m_selectedTransitionId = -1;
                    ImGui::CloseCurrentPopup();
                }
            }
            else
            {
                ImGui::TextUnformatted("No transition selected.");
            }
            ImGui::EndPopup();
        }
    }

  PortHover computeHoveredPort(const NodeGraphRenderContext& ctx) const
  {
    PortHover h;
    if (!ctx.editorHovered)
      return h;
    if (ctx.interaction.isOwnedByOtherThan(NodeGraphInteractionOwner::Other) && !m_pendingLinkActive)
      return h;

    // We intentionally place the port OUTSIDE the node rect so that clicking it doesn't
    // start ImNodes node dragging.
    constexpr float outerBand = 18.0f;     // how far outside the node we accept hover for a port
    constexpr float edgeBand = 16.0f;      // thickness for selecting an edge within the outer band
    constexpr float portOutset = 10.0f;    // how far outside the node edge the port is drawn

    for (const auto& kv : m_nodeRects)
    {
      const int nodeId = kv.first;
      const ImRect r = kv.second;

      // Only consider ports when hovering near but NOT inside the node.
      const ImRect expanded(ImVec2(r.Min.x - outerBand, r.Min.y - outerBand), ImVec2(r.Max.x + outerBand, r.Max.y + outerBand));
      if (!expanded.Contains(ctx.mouseScreen))
        continue;
      if (r.Contains(ctx.mouseScreen))
        continue;

      // Determine if mouse is near an edge in the OUTER band. If not, no port.
      const float x = ctx.mouseScreen.x;
      const float y = ctx.mouseScreen.y;
      const bool nearL = (x >= r.Min.x - outerBand) && (x <= r.Min.x + edgeBand);
      const bool nearR = (x <= r.Max.x + outerBand) && (x >= r.Max.x - edgeBand);
      const bool nearT = (y >= r.Min.y - outerBand) && (y <= r.Min.y + edgeBand);
      const bool nearB = (y <= r.Max.y + outerBand) && (y >= r.Max.y - edgeBand);
      if (!(nearL || nearR || nearT || nearB))
        continue;

      // Pick the closest edge *among the edges we are actually near*.
      float best = 1e9f;
      StateMachinePortSide side = StateMachinePortSide::West;
      if (nearL)
      {
        const float d = (x - r.Min.x);
        if (d < best) { best = d; side = StateMachinePortSide::West; }
      }
      if (nearR)
      {
        const float d = (r.Max.x - x);
        if (d < best) { best = d; side = StateMachinePortSide::East; }
      }
      if (nearT)
      {
        const float d = (y - r.Min.y);
        if (d < best) { best = d; side = StateMachinePortSide::North; }
      }
      if (nearB)
      {
        const float d = (r.Max.y - y);
        if (d < best) { best = d; side = StateMachinePortSide::South; }
      }

      // Place the port OUTSIDE the node edge and project along the edge.
      ImVec2 p = ctx.mouseScreen;
      p.x = std::max(r.Min.x, std::min(r.Max.x, p.x));
      p.y = std::max(r.Min.y, std::min(r.Max.y, p.y));
      switch (side)
      {
      case StateMachinePortSide::West:
        p.x = r.Min.x - portOutset;
        break;
      case StateMachinePortSide::East:
        p.x = r.Max.x + portOutset;
        break;
      case StateMachinePortSide::North:
        p.y = r.Min.y - portOutset;
        break;
      case StateMachinePortSide::South:
        p.y = r.Max.y + portOutset;
        break;
      }

      h.valid = true;
      h.nodeId = nodeId;
      h.side = side;
      h.pointScreen = p;
      h.offsetGrid = mouseOffsetGridOnNode(ctx.editorSpace, r, p);
      return h;
    }

    return h;
  }

  static void renderPortHover(ImDrawList* dl, const PortHover& h, bool pending)
  {
    if (!h.valid)
      return;
    const ImU32 col = pending ? IM_COL32(255, 220, 120, 255) : IM_COL32(180, 220, 255, 255);
    dl->AddCircleFilled(h.pointScreen, kEndpointRadius, IM_COL32(50, 50, 50, 255));
    dl->AddCircle(h.pointScreen, kEndpointRadius, col, 12, 2.0f);
  }
};

#endif // GLITTER_NODEGRAPH_STATEMACHINEVIEW_HPP












