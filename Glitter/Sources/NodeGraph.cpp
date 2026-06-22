//
// Created by subha on 15-03-2026.
//

#include "../Headers/NodeGraph/NodeGraph.hpp"
#include <imnodes.h>
#include <imgui.h>
#include <algorithm>
#include <unordered_set>

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
  // ImNodes hover includes focused windows; require actual window hover to avoid
  // click-through when multiple graph windows overlap.
  const bool windowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
  renderCtx.editorHovered = windowHovered && ImNodes::IsEditorHovered();
  renderCtx.leftClicked = renderCtx.editorHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
  renderCtx.leftDoubleClicked = renderCtx.editorHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
  renderCtx.leftDown = renderCtx.editorHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);
  renderCtx.leftReleased = renderCtx.editorHovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);

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

    // ImNodes hover/selection query APIs are only valid after EndNodeEditor().
    renderCtx.hoveredNodeId = -1;
    renderCtx.hoveredLinkId = -1;
    renderCtx.hoveredPinId = -1;
    renderCtx.activeAttributeId = -1;
    renderCtx.anyAttributeActive = false;
    (void)ImNodes::IsNodeHovered(&renderCtx.hoveredNodeId);
    (void)ImNodes::IsLinkHovered(&renderCtx.hoveredLinkId);
    (void)ImNodes::IsPinHovered(&renderCtx.hoveredPinId);
    renderCtx.anyAttributeActive = ImNodes::IsAnyAttributeActive(&renderCtx.activeAttributeId);

    int startAttr = -1;
    int endAttr = -1;
    if (ImNodes::IsLinkCreated(&startAttr, &endAttr))
    {
        auto nodeView = views.findView<NodeGraphNodesView>();
        nodeView->addLink(nodeGraphLinks, startAttr, endAttr);
    }

    int destroyedLinkId = -1;
    if (ImNodes::IsLinkDestroyed(&destroyedLinkId))
    {
        nodeGraphLinks.erase(
            std::remove_if(
                nodeGraphLinks.begin(),
                nodeGraphLinks.end(),
                [destroyedLinkId](const NodeGraphNodeLink& link) { return link.id() == destroyedLinkId; }),
            nodeGraphLinks.end());
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !ImGui::GetIO().WantTextInput)
    {
        const int selectedLinkCount = ImNodes::NumSelectedLinks();
        if (selectedLinkCount > 0)
        {
            std::vector<int> selectedLinkIds(static_cast<size_t>(selectedLinkCount));
            ImNodes::GetSelectedLinks(selectedLinkIds.data());

            nodeGraphLinks.erase(
                std::remove_if(
                    nodeGraphLinks.begin(),
                    nodeGraphLinks.end(),
                    [&selectedLinkIds](const NodeGraphNodeLink& link)
                    {
                        return std::find(selectedLinkIds.begin(), selectedLinkIds.end(), link.id()) != selectedLinkIds.end();
                    }),
                nodeGraphLinks.end());

            ImNodes::ClearLinkSelection();
        }

        const int selectedNodeCount = ImNodes::NumSelectedNodes();
        if (selectedNodeCount > 0)
        {
            std::vector<int> selectedNodeIds(static_cast<size_t>(selectedNodeCount));
            ImNodes::GetSelectedNodes(selectedNodeIds.data());

            deleteNodesByIds(selectedNodeIds);
            ImNodes::ClearNodeSelection();
        }
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

void NodeGraph::deleteNodesByIds(const std::vector<int>& nodeIds)
{
    if (nodeIds.empty())
        return;

    const std::unordered_set<int> nodeIdSet(nodeIds.begin(), nodeIds.end());

    // Remove regular node links connected to any deleted node pin.
    std::unordered_set<int> deletedNodeAttributeIds;
    for (const auto& node : nodes)
    {
        if (!node || nodeIdSet.find(node->id()) == nodeIdSet.end())
            continue;

        for (const auto& input : node->inputs())
            deletedNodeAttributeIds.insert(input.getId());
        for (const auto& output : node->outputs())
            deletedNodeAttributeIds.insert(output.getId());
        if (node->hasExecInput())
            deletedNodeAttributeIds.insert(node->getExecInput()->getId());
        if (node->hasExecOutput())
            deletedNodeAttributeIds.insert(node->getExecOutput()->getId());
    }

    nodeGraphLinks.erase(
        std::remove_if(
            nodeGraphLinks.begin(),
            nodeGraphLinks.end(),
            [&deletedNodeAttributeIds](const NodeGraphNodeLink& link)
            {
                return deletedNodeAttributeIds.find(link.startAttr()) != deletedNodeAttributeIds.end() ||
                       deletedNodeAttributeIds.find(link.endAttr()) != deletedNodeAttributeIds.end();
            }),
        nodeGraphLinks.end());

    nodes.erase(
        std::remove_if(
            nodes.begin(),
            nodes.end(),
            [&nodeIdSet](const std::unique_ptr<NodeGraphNode>& node)
            {
                return node && nodeIdSet.find(node->id()) != nodeIdSet.end();
            }),
        nodes.end());

    // Remove state-machine links connected to deleted state nodes.
    stateLinks.erase(
        std::remove_if(
            stateLinks.begin(),
            stateLinks.end(),
            [&nodeIdSet](const StateMachineLink& link)
            {
                return nodeIdSet.find(link.fromNodeId) != nodeIdSet.end() ||
                       nodeIdSet.find(link.toNodeId) != nodeIdSet.end();
            }),
        stateLinks.end());

    stateNodes.erase(
        std::remove_if(
            stateNodes.begin(),
            stateNodes.end(),
            [&nodeIdSet](const StateMachineNode& node)
            {
                return nodeIdSet.find(node.id) != nodeIdSet.end();
            }),
        stateNodes.end());
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
