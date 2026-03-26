//
// Created by subha on 15-03-2026.
//

#include "../Headers/NodeGraph/NodeGraph.hpp"
#include <imnodes.h>
#include <imgui.h>

#include "NodeGraph/NodeGraphContextMenu.hpp"
#include "NodeGraph/Views/NodeGraphCommentsView.hpp"
#include "NodeGraph/Views/NodeGraphNodesView.hpp"
#include "NodeGraph/Views/StateMachineView.hpp"

// imnodes doesn't expose a public EditorContextGet() API in this vendored version.
// To allow multiple NodeGraph instances to be drawn per-frame without leaking the
// current editor context across graphs, we track the last context we set.
static ImNodesEditorContext* g_lastImNodesEditorCtx = nullptr;

NodeGraph::NodeGraph()
: editorSpace(),
renderCtx(editorSpace, nodes, nodeGraphLinks, comments, stateNodes, stateLinks)
{
    editorCtx = ImNodes::EditorContextCreate();

    // Register default views. Their layer/priority controls draw order.
    views.emplaceView<NodeGraphCommentsView>(comments);
    views.emplaceView<NodeGraphNodesView>();
    views.emplaceView<StateMachineView>();

    auto& cm = views.emplaceView<NodeGraphContextMenu>();
    cm.setCallbacks(
        this,
        [](void* user, NodeTypes type, float x, float y)
        {
            auto* self = static_cast<NodeGraph*>(user);
            auto* nv = self->views.findView<NodeGraphNodesView>();
            if (!nv)
                return;

            nv->addNode(self->renderCtx.nodes, type, ImVec2(x, y));
        },
        [](void* user, const ImVec2& gridPos)
        {
            auto* self = static_cast<NodeGraph*>(user);
            auto* cv = self->views.findView<NodeGraphCommentsView>();
            if (!cv)
                return;

            cv->addComment(self->renderCtx.comments, gridPos);
        },
        [](void* user, const ImVec2& spawnPosScreen)
        {
            auto* self = static_cast<NodeGraph*>(user);
            auto* sv = self->views.findView<StateMachineView>();
            if (!sv)
                return;

            sv->addState(self->renderCtx.stateNodes, "State", spawnPosScreen);
            // If this is the first state, make it active by default.
            if (self->renderCtx.stateNodes.size() == 1)
                sv->setActive(self->renderCtx.stateNodes, self->renderCtx.stateNodes.front().id);
        });
}

NodeGraph::~NodeGraph()
{
    if (editorCtx)
    {
        // Ensure we don't leave this context as "current" when destroying.
        if (g_lastImNodesEditorCtx == editorCtx)
        {
            ImNodes::EditorContextSet(nullptr);
            g_lastImNodesEditorCtx = nullptr;
        }
        ImNodes::EditorContextFree(editorCtx);
        editorCtx = nullptr;
    }
}

void NodeGraph::drawUI()
{
    ImGui::Begin("node editor");

    drawUIEmbedded();

    ImGui::End();
}

void NodeGraph::drawUIEmbedded()
{
    // NOTE: Caller is responsible for Begin/End when embedding.

    // Isolation:
    // - Push a unique ImGui ID scope so any internal widgets/popups don't collide across graphs.
    // - Set this graph's ImNodes editor context so panning/selection/etc. are per-graph.
    ImGui::PushID(this);
    ImNodesEditorContext* prevCtx = g_lastImNodesEditorCtx;
    ImNodes::EditorContextSet(editorCtx);
    g_lastImNodesEditorCtx = editorCtx;

    ImNodes::BeginNodeEditor();

  // Capture per-frame input snapshot once; views should use renderCtx instead
  // of querying ImGui/ImNodes directly.
  renderCtx.mouseScreen = ImGui::GetMousePos();
  renderCtx.editorHovered = ImNodes::IsEditorHovered();
  renderCtx.leftClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
  renderCtx.leftDoubleClicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
  renderCtx.leftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
  renderCtx.leftReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

  // Interaction ownership:
  // - Keep owner sticky while mouse is down
  // - Release when mouse is released
  if (renderCtx.leftReleased)
    renderCtx.interaction.owner = NodeGraphInteractionOwner::None;
  renderCtx.interaction.resetPerFrame();

    // Capture stable mapping between grid-space and screen-space for this frame.
    editorSpace = NodeGraphEditorSpace::CaptureFromCurrentEditor();

    // Update the shared render context for this frame and let the registry
    // handle layering order (Background -> Content -> Overlay -> Popup).
    renderCtx.editorSpace = editorSpace;
    views.drawAll(renderCtx);

    // Optional caller/derived-class screen-space overlay UI.
    // This runs after all views have drawn and while the NodeGraph window is active.
    if (screenSpaceUiFn)
        screenSpaceUiFn(renderCtx, screenSpaceUiUser);

    ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
    ImNodes::EndNodeEditor();

    int startAttr = -1;
    int endAttr = -1;
    if (ImNodes::IsLinkCreated(&startAttr, &endAttr))
    {
        auto nodeView = views.findView<NodeGraphNodesView>();
        nodeView->addLink(nodeGraphLinks, startAttr, endAttr);
    }

    // Restore previous context (important when multiple graphs are drawn in one frame).
    ImNodes::EditorContextSet(prevCtx);
    g_lastImNodesEditorCtx = prevCtx;
    ImGui::PopID();
}

void NodeGraph::clearNodes()
{
    nodes.clear();
}

void NodeGraph::clearNodeGraphLinks()
{
    nodeGraphLinks.clear();
}

void NodeGraph::clearComments()
{
    comments.clear();
}

void NodeGraph::clearStateNodes()
{
    stateNodes.clear();
}

void NodeGraph::clearStateLinks()
{
    stateLinks.clear();
}
