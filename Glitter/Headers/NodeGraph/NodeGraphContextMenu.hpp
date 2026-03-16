//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODEGRAPH_CONTEXTMENU_HPP
#define GLITTER_NODEGRAPH_CONTEXTMENU_HPP

#include <imgui.h>
#include <imnodes.h>

#include "NodeGraphEditorSpace.hpp"

class NodeGraphContextMenu
{
public:
    bool showContextMenu = false;
    float contextMenuX = 0.0f;
    float contextMenuY = 0.0f;
    ImVec2 spawnPosScreen{0.0f, 0.0f};

    template <typename AddNodeFn, typename AddCommentFn>
    void draw(const NodeGraphEditorSpace& cache, AddNodeFn&& addNodeFn, AddCommentFn&& addCommentFn)
    {
        const bool editorHovered = ImNodes::IsEditorHovered();

        int hoveredNode = -1;
        int hoveredLink = -1;
        (void)ImNodes::IsNodeHovered(&hoveredNode);
        (void)ImNodes::IsLinkHovered(&hoveredLink);
        const bool hoveringNodeOrLink = (hoveredNode != -1) || (hoveredLink != -1);

        if (editorHovered && !hoveringNodeOrLink && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            showContextMenu = true;
            spawnPosScreen = ImGui::GetMousePos();
            contextMenuX = spawnPosScreen.x;
            contextMenuY = spawnPosScreen.y;
            ImGui::OpenPopup("NodeContextMenu");
        }

        if (ImGui::BeginPopup("NodeContextMenu"))
        {
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
                addNodeFn("Node", contextMenuX, contextMenuY);
                showContextMenu = false;
            }
            if (ImGui::MenuItem("Add Transform Node"))
            {
                addNodeFn("Transform", contextMenuX, contextMenuY);
                showContextMenu = false;
            }
            if (ImGui::MenuItem("Add Process Node"))
            {
                addNodeFn("Process", contextMenuX, contextMenuY);
                showContextMenu = false;
            }
            if (ImGui::MenuItem("Add Comment"))
            {
                const ImVec2 spawnGrid = cache.valid ? NodeGraphScreenToGrid(cache, spawnPosScreen)
                                                     : ImVec2(0.0f, 0.0f);
                addCommentFn(spawnGrid);
                showContextMenu = false;
            }

            ImGui::EndPopup();
        }
        else
        {
            showContextMenu = false;
        }
    }
};

#endif //GLITTER_NODEGRAPH_CONTEXTMENU_HPP


