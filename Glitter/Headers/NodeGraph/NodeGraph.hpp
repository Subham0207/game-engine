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

class NodeGraph
{
    public:
        NodeGraph();
        void drawUI();

    private:
        std::vector<NodeGraphNode> nodes;
        std::vector<CommentBox> comments;
        int nextNodeId;
        int nextCommentId;

        // Per-frame cached editor space mapping
        NodeGraphEditorSpace editorSpace;

        // Components
        NodeGraphNodesView nodesView;
        NodeGraphCommentsView commentsView;
        NodeGraphContextMenu contextMenu;

        void addNode(const std::string& name, float x, float y);
        void addComment(const ImVec2& posGrid);
};


#endif //GLITTER_NODEGRAPH_HPP