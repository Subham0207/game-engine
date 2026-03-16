//
// Created by subha on 15-03-2026.
//

#ifndef GLITTER_NODEGRAPH_HPP
#define GLITTER_NODEGRAPH_HPP

#include <vector>
#include <string>
#include <imgui.h>

#include "NodeGraphNode.hpp"

#include "CommentBox.hpp"
#include "NodeGraphEditorSpace.hpp"
#include "NodeGraphNodesView.hpp"
#include "NodeGraphCommentsView.hpp"
#include "NodeGraphContextMenu.hpp"
#include "NodeGraphRenderContext.hpp"
#include "NodeGraphViewRegistry.hpp"

class NodeGraph
{
    public:
        NodeGraph();
        void drawUI();

    private:
        std::vector<NodeGraphNode> nodes;
        std::vector<CommentBox> comments;

        // Per-frame cached editor space mapping
        NodeGraphEditorSpace editorSpace;

        // Layered UI system
        NodeGraphViewRegistry views;
        NodeGraphRenderContext renderCtx{editorSpace, nodes, comments};

};


#endif //GLITTER_NODEGRAPH_HPP