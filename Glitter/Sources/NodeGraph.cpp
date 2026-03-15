//
// Created by subha on 15-03-2026.
//

#include "../Headers/NodeGraph/NodeGraph.hpp"
#include <imnodes.h>
#include <imgui.h>

NodeGraph::NodeGraph()
    : nextNodeId(0), showContextMenu(false), contextMenuX(0.0f), contextMenuY(0.0f), spawnPosScreen(0.0f, 0.0f)
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

    // Cache editor origin in screen space for this frame. Using GetCursorScreenPos() inside
    // context-menu logic can be unstable (cursor moves as widgets are drawn).
    // We can derive the editor origin by comparing a known grid-space point to its screen-space.
    // Node id 0 is not special here; we just use a constant grid-space sample.
    // (If you already know a better API in your imnodes version, use it instead.)

    drawNodes();
    handleContextMenu();

    ImNodes::EndNodeEditor();

    ImGui::End();
}








