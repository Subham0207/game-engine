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
#include "../NodeGraphIdRanges.hpp"
#include "../Components/StateMachineNode.hpp"
#include "../Components/StateMachineLink.hpp"

class StatemachineFlowScript;
// View: draws and edits a prototype state-machine graph.
// - State nodes are ImNodes nodes (we rely on ImNodes for node selection + movement).
// - Links are fully custom rendered & interactive, with ports that can attach
//   anywhere on the node body (any direction, multiple links can share the same point).
class StateMachineView final : public INodeGraphView
{
public:
    [[nodiscard]] NodeGraphLayer layer() const override { return NodeGraphLayer::Content; }

    // Draw after basic node view by default.
    // (Smaller number draws earlier within a layer.)
    [[nodiscard]] int priority() const override { return 5; }

    // IMPORTANT: State-machine nodes are drawn in the same ImNodes editor as regular NodeGraph nodes.
    // ImNodes requires all node IDs to be unique across the entire editor. We allocate IDs from a
    // dedicated reserved range to avoid collisions and to keep room for future UI element types.
    int nextStateId = NodeGraphIdBase(NodeGraphElementIdBase::StateMachineNode);
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
        auto& links = ctx.stateLinks;

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

            // Rename UX: double-click the name to start editing.
            // Commit on Enter or when the input loses focus.
            if (m_renamingNodeId == n.id)
            {
                ImGui::SetNextItemWidth(180.0f);
                const std::string label = std::string("##SM_Rename_") + std::to_string(n.id);
                const bool enter = ImGui::InputText(label.c_str(), m_renameBuf, sizeof(m_renameBuf), ImGuiInputTextFlags_EnterReturnsTrue);
                const bool deactivated = ImGui::IsItemDeactivatedAfterEdit();

                if (enter || deactivated)
                {
                    if (m_renameBuf[0] != '\0')
                        n.name = m_renameBuf;
                    m_renamingNodeId = -1;
                    m_renameBuf[0] = '\0';
                }
                else
                {
                    // Keep keyboard focus while renaming.
                    if (!ImGui::IsItemActive())
                        ImGui::SetKeyboardFocusHere(-1);
                }
            }
            else
            {
                ImGui::TextUnformatted(n.name.c_str());
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    m_renamingNodeId = n.id;
                    strncpy_s(m_renameBuf, sizeof(m_renameBuf), n.name.c_str(), sizeof(m_renameBuf) - 1);
                }
            }

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

    void setFlowScriptRef(StatemachineFlowScript* ref){mSmflowscriptRef = ref;}

private:

    StatemachineFlowScript* mSmflowscriptRef = nullptr;

    struct LinkDragState
    {
        bool active = false;
        int linkId = -1;
        bool draggingFromPort = true; // true=dragging from-port, false=to-port
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

  struct PortInteractionFrame
  {
    PortHover hovered;
    bool portClickedThisFrame = false;
    bool portReleasedThisFrame = false;
    PortHover clicked;
  };

  struct LinkHitResult
  {
    int hoveredTransition = -1;
    bool hoveredFrom = false;
    float bestPortDist = 1e9f;
    float bestLinkDist = 1e9f;
  };

    // Converts a saved state-machine link port position into screen-space.
    //
    // Why this exists:
    // - We persist link attachment points as a GRID-SPACE offset relative to a state-machine node.
    //   (See StateMachineLink::{fromOffsetGrid,toOffsetGrid}).
    // - At render time we need the actual pixel position on screen, accounting for editor pan/zoom.
    //
    // How it works:
    // 1) Convert the node's top-left corner from SCREEN -> GRID.
    // 2) Add the persisted relative port offset (GRID).
    // 3) Convert the resulting absolute port position from GRID -> SCREEN.
    //
    // Params:
    // - space: current editor transform (pan/zoom) used for GRID<->SCREEN conversions.
    // - stateMachineNodeRect: the node rectangle in SCREEN space for the node the port belongs to.
    // - portGridOffsetRelSm: the port offset in GRID space, relative to the node's top-left.
    static ImVec2 portToScreen(const NodeGraphEditorSpace& space, const ImRect& stateMachineNodeRect, const ImVec2& portGridOffsetRelSm)
    {
        const ImVec2 nodeMinGrid = NodeGraphScreenToGrid(space, stateMachineNodeRect.Min);
        const ImVec2 pGrid(nodeMinGrid.x + portGridOffsetRelSm.x, nodeMinGrid.y + portGridOffsetRelSm.y);
        return NodeGraphGridToScreen(space, pGrid);
    }

    static ImVec2 mouseOffsetGridOnNode(const NodeGraphEditorSpace& space, const ImRect& nodeRectScreen, const ImVec2& mouseScreen)
    {
        const ImVec2 mouseGrid = NodeGraphScreenToGrid(space, mouseScreen);
        const ImVec2 nodeMinGrid = NodeGraphScreenToGrid(space, nodeRectScreen.Min);
        return ImVec2(mouseGrid.x - nodeMinGrid.x, mouseGrid.y - nodeMinGrid.y);
    }

    static StateMachineLink* findLink(std::vector<StateMachineLink>& links, int id)
    {
        for (auto& t : links)
        {
            if (t.id == id)
                return &t;
        }
        return nullptr;
    }

    static void eraseLink(std::vector<StateMachineLink>& links, int id)
    {
        links.erase(std::remove_if(links.begin(), links.end(), [&](const StateMachineLink& t) { return t.id == id; }), links.end());
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
    static constexpr float kPortRadius = 6.0f;
    static constexpr float kPortHitRadius = 10.0f;
    static constexpr float kBezierTangent = 80.0f;
    static constexpr float kLinkHitDist = 8.0f;

    // We reuse StateMachineLink's fromSide/toSide fields as a compact storage for
    // the nearest-edge direction of the port (helps bezier direction). The actual
    // port position is stored persistently in StateMachineLink::fromOffsetGrid/toOffsetGrid.

    // Per-frame node rect cache in screen-space.
    std::unordered_map<int, ImRect> m_nodeRects;

  LinkDragState m_dragLink;
    int m_selectedLinkId = -1;
	bool m_pendingLinkActive = false;
  PortHover m_pendingStartPort;

    // Rename UI state.
    int m_renamingNodeId = -1;
    char m_renameBuf[128] = {0};

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

    bool linkPortScreenPos(const NodeGraphEditorSpace& space, const StateMachineLink& t, bool from, ImVec2& outP) const
    {
        const int nodeId = from ? t.fromNodeId : t.toNodeId;
        auto nr = m_nodeRects.find(nodeId);
        if (nr == m_nodeRects.end())
            return false;
        const ImVec2 offset = from ? t.fromOffsetGrid : t.toOffsetGrid;
        outP = portToScreen(space, nr->second, offset);
        return true;
    }

    void ensureLinkPortsInitialized(const NodeGraphEditorSpace& space, std::vector<StateMachineLink>& links)
    {
        for (auto& t : links)
        {
            // If ports missing, initialize them to a reasonable default on the border
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

    void drawAndInteractLinks(NodeGraphRenderContext& ctx, std::vector<StateMachineNode>& nodes, std::vector<StateMachineLink>& links)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 mouse = ctx.mouseScreen;

        ensureLinkPortsInitialized(ctx.editorSpace, links);

        const PortInteractionFrame portFrame = updatePortHoverAndCaptureInput(ctx, dl);
        updatePendingLinkCreation(ctx, portFrame, mouse, dl, links);
        const LinkHitResult hit = hitTestLinksAndPorts(ctx, links);
        handleLinkClickInteractions(ctx, hit);
        updatePortDrag(ctx, links);

        renderAllLinks(dl, ctx, links, hit.hoveredTransition);
        renderLinkEditPopup(links);
    }

    PortInteractionFrame updatePortHoverAndCaptureInput(NodeGraphRenderContext& ctx, ImDrawList* dl)
    {
        PortInteractionFrame f;

        // --- Port hover logic (for starting/ending links)
        f.hovered = computeHoveredPort(ctx);

        // If the mouse is over a port (or we are dragging a pending link), aggressively capture
        // mouse input to prevent ImNodes from starting marquee selection.
        // Note: SetNextFrameWantCaptureMouse alone is not enough for ImNodes; we need to also
        // swallow the drag by capturing while mouse is down.
        const bool wantPortInteraction = f.hovered.valid || m_pendingLinkActive;
        if (wantPortInteraction)
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetNextFrameWantCaptureMouse(true);
            if (ctx.leftDown)
                ImGui::SetNextFrameWantCaptureMouse(true);

            // Create an invisible hit-box over the port region so ImGui consumes the click/drag.
            // This prevents ImNodes from starting marquee selection in the background.
            if (f.hovered.valid)
            {
                ImGui::SetCursorScreenPos(ImVec2(f.hovered.pointScreen.x - kPortHitRadius, f.hovered.pointScreen.y - kPortHitRadius));
                ImGui::InvisibleButton("##SM_PortHit", ImVec2(kPortHitRadius * 2.0f, kPortHitRadius * 2.0f));
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                {
                    f.portClickedThisFrame = true;
                    f.clicked = f.hovered;
                }
                // When dragging a pending link, finish it on mouse-release over a target port.
                // This mirrors typical node editor UX: click start, drag, release to connect.
                if (m_pendingLinkActive && ImGui::IsItemDeactivated() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                {
                    f.portReleasedThisFrame = true;
                    f.clicked = f.hovered;
                }
            }
        }

        renderPortHover(dl, f.hovered, m_pendingLinkActive);
        return f;
    }

    void updatePendingLinkCreation(NodeGraphRenderContext& ctx, const PortInteractionFrame& f, const ImVec2& mouse,
                                   ImDrawList* dl, std::vector<StateMachineLink>& links)
    {
        bool startedPendingThisFrame = false;

        // Cancel pending start if user clicks away.
        // Do not cancel on the same frame we clicked a port hitbox; that click is handled below.
        if (m_pendingLinkActive && ctx.leftClicked && !f.hovered.valid && !f.portClickedThisFrame)
        {
            m_pendingLinkActive = false;
            m_pendingStartPort = PortHover{};
        }

        // Start a new link ONLY when clicking a hovered port.
        // We claim ownership with a higher priority than ImNodes so that even if ImNodes also
        // wants the mouse this frame, our port interaction wins.
        if (!m_pendingLinkActive && f.portClickedThisFrame && f.clicked.valid && ctx.interaction.tryClaim(NodeGraphInteractionOwner::Other, 1000))
        {
            m_pendingLinkActive = true;
            m_pendingStartPort = f.clicked;
            startedPendingThisFrame = true;
        }

        // If we have a pending start, drag a preview line.
        if (m_pendingLinkActive)
        {
            const ImU32 previewCol = IM_COL32(200, 200, 220, 200);
            const ImVec2 p0 = m_pendingStartPort.pointScreen;
            const ImVec2 d0 = sideDir(m_pendingStartPort.side);
            const ImVec2 c1(p0.x + d0.x * kBezierTangent, p0.y + d0.y * kBezierTangent);
            const ImVec2 c2(mouse.x, mouse.y);
            dl->AddBezierCubic(p0, c1, c2, mouse, previewCol, 2.0f);
        }

        // Finish: if pending and clicking/releasing on a hovered port on another node, create transition.
        // Guard against the click that started the pending link; otherwise we'd start+finish immediately.
        if (m_pendingLinkActive && !startedPendingThisFrame && (f.portClickedThisFrame || f.portReleasedThisFrame) && f.clicked.valid)
        {
            // If user clicked the same port again, treat it as cancel.
            if (f.clicked.nodeId == m_pendingStartPort.nodeId && f.clicked.side == m_pendingStartPort.side)
            {
                m_pendingLinkActive = false;
                m_pendingStartPort = PortHover{};
                ctx.interaction.release(NodeGraphInteractionOwner::Other);
            }
            else
            {
                StateMachineLink tr;
                tr.id = nextTransitionId++;
                tr.fromNodeId = m_pendingStartPort.nodeId;
                tr.toNodeId = f.clicked.nodeId;
                tr.fromSide = m_pendingStartPort.side;
                tr.toSide = f.clicked.side;
                tr.fromOffsetGrid = m_pendingStartPort.offsetGrid;
                tr.toOffsetGrid = f.clicked.offsetGrid;
                tr.condition = "";

                links.emplace_back(std::move(tr));
                m_pendingLinkActive = false;
                m_pendingStartPort = PortHover{};
                ctx.interaction.release(NodeGraphInteractionOwner::Other);
            }
        }
    }

    LinkHitResult hitTestLinksAndPorts(const NodeGraphRenderContext& ctx, std::vector<StateMachineLink>& links) const
    {
        const ImVec2 mouse = ctx.mouseScreen;
        LinkHitResult r;

        for (auto& t : links)
        {
            ImVec2 pFrom, pTo;
            if (!linkPortScreenPos(ctx.editorSpace, t, true, pFrom) || !linkPortScreenPos(ctx.editorSpace, t, false, pTo))
                continue;

            const float df = sqrtf((mouse.x - pFrom.x) * (mouse.x - pFrom.x) + (mouse.y - pFrom.y) * (mouse.y - pFrom.y));
            const float dt = sqrtf((mouse.x - pTo.x) * (mouse.x - pTo.x) + (mouse.y - pTo.y) * (mouse.y - pTo.y));
            if (df < r.bestPortDist && df <= kPortHitRadius)
            {
                r.bestPortDist = df;
                r.hoveredTransition = t.id;
                r.hoveredFrom = true;
            }
            if (dt < r.bestPortDist && dt <= kPortHitRadius)
            {
                r.bestPortDist = dt;
                r.hoveredTransition = t.id;
                r.hoveredFrom = false;
            }

            // Bezier hit test
            const ImVec2 d0 = sideDir(t.fromSide);
            const ImVec2 d1 = sideDir(t.toSide);
            const ImVec2 c1(pFrom.x + d0.x * kBezierTangent, pFrom.y + d0.y * kBezierTangent);
            const ImVec2 c2(pTo.x + d1.x * kBezierTangent, pTo.y + d1.y * kBezierTangent);
            const float dist = hitTestBezier(mouse, pFrom, c1, c2, pTo);
            if (dist < r.bestLinkDist)
            {
                r.bestLinkDist = dist;
                if (r.bestPortDist > kPortHitRadius)
                {
                    // only consider link hover if not on a port
                    r.hoveredTransition = (dist <= kLinkHitDist) ? t.id : r.hoveredTransition;
                }
            }
        }
        return r;
    }

    void handleLinkClickInteractions(NodeGraphRenderContext& ctx, const LinkHitResult& hit)
    {
        // Begin interactions
        // - Click on port: drag that port (reconnect)
        // - Click on link: select + open popup for condition

        const bool allow = ctx.interaction.canInteract(NodeGraphInteractionOwner::Other);
        if (allow && ctx.leftClicked)
        {
            if (hit.hoveredTransition != -1 && hit.bestPortDist <= kPortHitRadius)
            {
                // Drag port
                ctx.interaction.tryClaim(NodeGraphInteractionOwner::Other, 30);
                m_dragLink.active = true;
                m_dragLink.linkId = hit.hoveredTransition;
                m_dragLink.draggingFromPort = hit.hoveredFrom;
                m_dragLink.mouseScreen = ctx.mouseScreen;
            }
            else
            {
                if (hit.hoveredTransition != -1 && hit.bestLinkDist <= kLinkHitDist)
                {
                    // Select link
                    m_selectedLinkId = hit.hoveredTransition;
                    ImGui::OpenPopup("SM_LinkEdit");
                }
            }
        }
    }

    void updatePortDrag(NodeGraphRenderContext& ctx, std::vector<StateMachineLink>& links)
    {
        const ImVec2 mouse = ctx.mouseScreen;

        // Update drag
        if (m_dragLink.active)
        {
            if (ctx.interaction.isOwnedBy(NodeGraphInteractionOwner::Other))
            {
                StateMachineLink* tr = findLink(links, m_dragLink.linkId);
                if (tr)
                {
                    int nodeId = -1;
                    ImRect r;
                    if (getNodeAt(mouse, nodeId, r))
                    {
                        const StateMachinePortSide side = computeNearestSide(r, mouse);
                        if (m_dragLink.draggingFromPort)
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
                        const int currentNodeId = m_dragLink.draggingFromPort ? tr->fromNodeId : tr->toNodeId;
                        auto nr = m_nodeRects.find(currentNodeId);
                        if (nr != m_nodeRects.end())
                        {
                            // Clamp to node bounds so it doesn't explode.
                            ImVec2 clamped = mouse;
                            clamped.x = std::max(nr->second.Min.x, std::min(nr->second.Max.x, clamped.x));
                            clamped.y = std::max(nr->second.Min.y, std::min(nr->second.Max.y, clamped.y));
                            if (m_dragLink.draggingFromPort)
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
                StateMachineLink* tr = findLink(links, m_dragLink.linkId);
                bool ok = false;
                if (tr)
                {
                    ok = (m_nodeRects.find(tr->fromNodeId) != m_nodeRects.end()) && (m_nodeRects.find(tr->toNodeId) != m_nodeRects.end());
                    if (!ok)
                        eraseLink(links, tr->id);
                }
                m_dragLink = LinkDragState{};
                ctx.interaction.release(NodeGraphInteractionOwner::Other);
            }
        }
    }

    void renderAllLinks(ImDrawList* dl, const NodeGraphRenderContext& ctx, std::vector<StateMachineLink>& links, int hoveredTransition)
    {
        // Render links + ports (after interaction updates)
        for (auto& t : links)
        {
            ImVec2 pFrom, pTo;
            if (!linkPortScreenPos(ctx.editorSpace, t, true, pFrom) || !linkPortScreenPos(ctx.editorSpace, t, false, pTo))
                continue;

            const bool selected = (t.id == m_selectedLinkId);
            const bool hovered = (t.id == hoveredTransition);
            const ImU32 col = selected ? IM_COL32(255, 220, 120, 255) : hovered ? IM_COL32(180, 220, 255, 255) : IM_COL32(200, 200, 200, 255);
            const float thick = selected ? 3.0f : hovered ? 2.5f : 2.0f;

            const ImVec2 d0 = sideDir(t.fromSide);
            const ImVec2 d1 = sideDir(t.toSide);
            const ImVec2 c1(pFrom.x + d0.x * kBezierTangent, pFrom.y + d0.y * kBezierTangent);
            const ImVec2 c2(pTo.x + d1.x * kBezierTangent, pTo.y + d1.y * kBezierTangent);

            dl->AddBezierCubic(pFrom, c1, c2, pTo, col, thick);

            // Direction arrow (from -> to) near the middle.
            // Use the actual cubic bezier tangent at a fixed parameter so the arrow
            // doesn't flip as nodes move.
            {
                constexpr float tMid = 0.5f;
                const float u = 1.0f - tMid;

                // Bezier point B(t)
                const ImVec2 mid(
                    (u * u * u) * pFrom.x + (3.0f * u * u * tMid) * c1.x + (3.0f * u * tMid * tMid) * c2.x + (tMid * tMid * tMid) * pTo.x,
                    (u * u * u) * pFrom.y + (3.0f * u * u * tMid) * c1.y + (3.0f * u * tMid * tMid) * c2.y + (tMid * tMid * tMid) * pTo.y);

                // Bezier derivative B'(t)
                ImVec2 dir(
                    (3.0f * u * u) * (c1.x - pFrom.x) + (6.0f * u * tMid) * (c2.x - c1.x) + (3.0f * tMid * tMid) * (pTo.x - c2.x),
                    (3.0f * u * u) * (c1.y - pFrom.y) + (6.0f * u * tMid) * (c2.y - c1.y) + (3.0f * tMid * tMid) * (pTo.y - c2.y));

                const float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
                if (len > 0.0001f)
                {
                    dir.x /= len;
                    dir.y /= len;
                    const ImVec2 nrm(-dir.y, dir.x);

                    const float arrowLen = 12.0f;
                    const float arrowWidth = 6.0f;
                    const ImVec2 tip(mid.x + dir.x * (arrowLen * 0.5f), mid.y + dir.y * (arrowLen * 0.5f));
                    const ImVec2 base(mid.x - dir.x * (arrowLen * 0.5f), mid.y - dir.y * (arrowLen * 0.5f));
                    const ImVec2 left(base.x + nrm.x * arrowWidth, base.y + nrm.y * arrowWidth);
                    const ImVec2 right(base.x - nrm.x * arrowWidth, base.y - nrm.y * arrowWidth);
                    dl->AddTriangleFilled(tip, left, right, col);
                }
            }

            // Ports
            dl->AddCircleFilled(pFrom, kPortRadius, IM_COL32(60, 60, 60, 255));
            dl->AddCircle(pFrom, kPortRadius, col, 12, 2.0f);
            dl->AddCircleFilled(pTo, kPortRadius, IM_COL32(60, 60, 60, 255));
            dl->AddCircle(pTo, kPortRadius, col, 12, 2.0f);

            // Quick condition label near the middle.
            const ImVec2 mid((pFrom.x + pTo.x) * 0.5f, (pFrom.y + pTo.y) * 0.5f);
            if (!t.condition.empty())
                dl->AddText(ImVec2(mid.x + 6.0f, mid.y + 6.0f), IM_COL32(240, 240, 240, 255), t.condition.c_str());
        }
    }

    void EditCondition(StateMachineLink* selectedLink) const;

    void renderLinkEditPopup(std::vector<StateMachineLink>& links)
    {
        // Link edit popup
        if (ImGui::BeginPopup("SM_LinkEdit"))
        {
            StateMachineLink* tr = findLink(links, m_selectedLinkId);
            if (tr)
            {
                ImGui::Text("Link %d", tr->id);

                EditCondition(tr);

                if (ImGui::Button("Delete"))
                {
                    eraseLink(links, tr->id);
                    m_selectedLinkId = -1;
                    ImGui::CloseCurrentPopup();
                }
            }
            else
            {
                ImGui::TextUnformatted("No link selected.");
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
    dl->AddCircleFilled(h.pointScreen, kPortRadius, IM_COL32(50, 50, 50, 255));
    dl->AddCircle(h.pointScreen, kPortRadius, col, 12, 2.0f);
  }
};

#endif // GLITTER_NODEGRAPH_STATEMACHINEVIEW_HPP












