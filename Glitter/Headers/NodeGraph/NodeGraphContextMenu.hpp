//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODEGRAPH_CONTEXTMENU_HPP
#define GLITTER_NODEGRAPH_CONTEXTMENU_HPP

#include <imgui.h>
#include <imnodes.h>

#include "NodeGraphEditorSpace.hpp"
#include "Views/INodeGraphView.hpp"
#include "NodeGraphRenderContext.hpp"

class NodeGraphContextMenu final : public INodeGraphView
{
public:
    bool showContextMenu = false;
    float contextMenuX = 0.0f;
    float contextMenuY = 0.0f;
    ImVec2 spawnPosScreen{0.0f, 0.0f};

    [[nodiscard]] NodeGraphLayer layer() const override { return NodeGraphLayer::Popup; }

    using AddNodeFn = void (*)(void* user, const std::string& base, float x, float y);
    using AddCommentFn = void (*)(void* user, const ImVec2& gridPos);
    using AddStateNodeFn = void (*)(void* user, const ImVec2& spawnPosScreen);

    void setCallbacks(void* user, AddNodeFn addNode, AddCommentFn addComment, AddStateNodeFn addStateNode)
    {
        m_user = user;
        m_addNode = addNode;
        m_addComment = addComment;
		m_addStateNode = addStateNode;
    }

    void draw(NodeGraphRenderContext& ctx) override
    {
        const NodeGraphEditorSpace& cache = ctx.editorSpace;
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
            // While a popup is open, block other interaction so clicks don't leak.
            ctx.interaction.tryClaim(NodeGraphInteractionOwner::ContextMenu, 100);

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
                if (m_addNode)
                    m_addNode(m_user, "Node", contextMenuX, contextMenuY);
                showContextMenu = false;
            }
            if (ImGui::MenuItem("Add Transform Node"))
            {
                if (m_addNode)
                    m_addNode(m_user, "Transform", contextMenuX, contextMenuY);
                showContextMenu = false;
            }
            if (ImGui::MenuItem("Add Process Node"))
            {
                if (m_addNode)
                    m_addNode(m_user, "Process", contextMenuX, contextMenuY);
                showContextMenu = false;
            }
            if (ImGui::MenuItem("Add Comment"))
            {
                const ImVec2 spawnGrid = cache.valid ? NodeGraphScreenToGrid(cache, spawnPosScreen)
                                                     : ImVec2(0.0f, 0.0f);
                if (m_addComment)
                    m_addComment(m_user, spawnGrid);
                showContextMenu = false;
            }

			ImGui::Separator();
			ImGui::Text("State Machine");
			if (ImGui::MenuItem("Add State"))
			{
				if (m_addStateNode)
					m_addStateNode(m_user, spawnPosScreen);
				showContextMenu = false;
			}

            ImGui::EndPopup();
        }
        else
        {
            showContextMenu = false;
        }
    }

private:
    void* m_user = nullptr;
    AddNodeFn m_addNode = nullptr;
    AddCommentFn m_addComment = nullptr;
	AddStateNodeFn m_addStateNode = nullptr;
};

#endif //GLITTER_NODEGRAPH_CONTEXTMENU_HPP



