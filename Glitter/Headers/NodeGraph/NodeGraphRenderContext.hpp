//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODEGRAPH_RENDERCONTEXT_HPP
#define GLITTER_NODEGRAPH_RENDERCONTEXT_HPP

#include <vector>

#include "NodeGraphEditorSpace.hpp"
#include "NodeGraphNode.hpp"
#include "CommentBox.hpp"

// Shared data passed to each registered NodeGraph view for this frame.
// Keep it small and stable so adding new view types is easy.
struct NodeGraphRenderContext
{
	// Stable mapping between editor grid-space and screen-space for this frame.
	NodeGraphEditorSpace editorSpace;

	// Graph model containers.
	std::vector<NodeGraphNode>& nodes;
	std::vector<CommentBox>& comments;
};

#endif //GLITTER_NODEGRAPH_RENDERCONTEXT_HPP

