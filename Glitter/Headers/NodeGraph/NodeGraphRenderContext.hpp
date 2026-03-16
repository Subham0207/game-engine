//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODEGRAPH_RENDERCONTEXT_HPP
#define GLITTER_NODEGRAPH_RENDERCONTEXT_HPP

#include <vector>

#include <imgui.h>

#include "NodeGraphEditorSpace.hpp"
#include "NodeGraphNode.hpp"
#include "CommentBox.hpp"

// Shared data passed to each registered NodeGraph view for this frame.
// Keep it small and stable so adding new view types is easy.

enum class NodeGraphInteractionOwner : int
{
	None = 0,
	ImNodes,      // generic "the imnodes editor is interacting with something"
	Comments,
	ContextMenu,
	Other
};

struct NodeGraphInteractionState
{
	NodeGraphInteractionOwner owner = NodeGraphInteractionOwner::None;
	int ownerPriority = 0;

	void resetPerFrame()
	{
		// Keep owner sticky across frames while mouse is held; only clear in NodeGraph when appropriate.
		// This reset exists so we can extend this later with per-frame diagnostics.
	}

	[[nodiscard]] bool canInteract(NodeGraphInteractionOwner who) const
	{
		return owner == NodeGraphInteractionOwner::None || owner == who;
	}

	[[nodiscard]] bool isOwnedBy(NodeGraphInteractionOwner who) const
	{
		return owner == who;
	}

	[[nodiscard]] bool isOwnedByOtherThan(NodeGraphInteractionOwner who) const
	{
		return owner != NodeGraphInteractionOwner::None && owner != who;
	}

	bool tryClaim(NodeGraphInteractionOwner who, int priority)
	{
		if (owner == NodeGraphInteractionOwner::None)
		{
			owner = who;
			ownerPriority = priority;
			return true;
		}
		if (owner == who)
			return true;
		if (priority > ownerPriority)
		{
			owner = who;
			ownerPriority = priority;
			return true;
		}
		return false;
	}

	void release(NodeGraphInteractionOwner who)
	{
		if (owner == who)
		{
			owner = NodeGraphInteractionOwner::None;
			ownerPriority = 0;
		}
	}
};

struct NodeGraphRenderContext
{
	// Stable mapping between editor grid-space and screen-space for this frame.
	NodeGraphEditorSpace editorSpace;

	// Per-frame editor input snapshot so views don't have to query ImGui/ImNodes repeatedly.
	// These are captured by NodeGraph once per frame.
	bool editorHovered = false;
	bool leftClicked = false;
	bool leftDoubleClicked = false;
	bool leftDown = false;
	bool leftReleased = false;
	ImVec2 mouseScreen{0.0f, 0.0f};

	// Exclusive input/interaction ownership across views.
	NodeGraphInteractionState interaction;

	// Graph model containers.
	std::vector<NodeGraphNode>& nodes;
	std::vector<CommentBox>& comments;
};

#endif //GLITTER_NODEGRAPH_RENDERCONTEXT_HPP

