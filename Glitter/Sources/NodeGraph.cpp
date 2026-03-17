//
// Created by subha on 15-03-2026.
//

#include "../Headers/NodeGraph/NodeGraph.hpp"
#include <imnodes.h>
#include <imgui.h>

#include "NodeGraph/NodeGraphContextMenu.hpp"
#include "NodeGraph/Views/NodeGraphCommentsView.hpp"
#include "NodeGraph/Views/NodeGraphNodesView.hpp"

NodeGraph::NodeGraph()
    : editorSpace()
{
    // Register default views. Their layer/priority controls draw order.
    views.emplaceView<NodeGraphCommentsView>(comments);
    views.emplaceView<NodeGraphNodesView>();

    auto& cm = views.emplaceView<NodeGraphContextMenu>();
    cm.setCallbacks(
        this,
        [](void* user, const std::string& base, float x, float y) {
            auto* self = static_cast<NodeGraph*>(user);
            auto* nv = self->views.findView<NodeGraphNodesView>();
            if (!nv)
                return;

            const std::string name = base + " " + std::to_string(nv->nextNodeId);
            nv->addNode(self->renderCtx.nodes, name, ImVec2(x, y));
        },
        [](void* user, const ImVec2& gridPos) {
            auto* self = static_cast<NodeGraph*>(user);
            auto* cv = self->views.findView<NodeGraphCommentsView>();
            if (!cv)
                return;

            cv->addComment(self->renderCtx.comments, gridPos);
        });
}

void NodeGraph::drawUI()
{
    ImGui::Begin("node editor");

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

    ImNodes::EndNodeEditor();

    ImGui::End();
}








