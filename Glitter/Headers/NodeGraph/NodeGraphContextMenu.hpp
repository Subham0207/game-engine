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
#include "Components/NodeGraphNodes/NodeTypes.hpp"
#include "Components/NodeGraphNodes/BinaryOperators/Add.hpp"

using AddNode = NodeGraphComponents::Node::Add;

class NodeGraphContextMenu final : public INodeGraphView
{
public:
    bool showContextMenu = false;
    float contextMenuX = 0.0f;
    float contextMenuY = 0.0f;
    ImVec2 spawnPosScreen{0.0f, 0.0f};

    [[nodiscard]] NodeGraphLayer layer() const override { return NodeGraphLayer::Popup; }

    using AddNodeFn = void (*)(void* user, NodeTypes type, float x, float y);
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
        const bool editorHovered = ctx.editorHovered;

        const bool hoveringNodeOrLink = (ctx.hoveredNodeId != -1) || (ctx.hoveredLinkId != -1);

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

            ImGui::Text("Binary Operators");
            ImGui::Separator();

            if (ImGui::MenuItem("Add"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::Add, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("Subtract"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::Subtract, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("Multiply"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::Multiply, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("Divide"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::Divide, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("Modulo"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::Modulo, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("LessThan"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::LessThan, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("GreaterThan"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::GreaterThan, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("EqualsTo"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::EqualsTo, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("NotEqualsTo"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::NotEqualsTo, contextMenuX, contextMenuY);

            }

            ImGui::Separator();
            ImGui::Text("Data Types");
            ImGui::Separator();

            if (ImGui::MenuItem("Integer"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::Integer, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("Boolean"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::Boolean, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("Get Variable"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::GetVariable, contextMenuX, contextMenuY);

            }

            ImGui::Separator();
            ImGui::Text("Keywords");
            ImGui::Separator();

            if (ImGui::MenuItem("Function"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::Function, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("Print"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::Print, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("Return"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::Return, contextMenuX, contextMenuY);

            }
            if (ImGui::MenuItem("Variable Declaration"))
            {
                if (m_addNode)
                    m_addNode(m_user, NodeTypes::VariableDeclaration, contextMenuX, contextMenuY);

            }

            ImGui::Separator();
            ImGui::Text("Others");
            ImGui::Separator();

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
            ImGui::Separator();
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





