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
        bool showContextMenu;
        float contextMenuX;
        float contextMenuY;
        // Screen space spawn position (captured on right-click), used to place new nodes.
        ImVec2 spawnPosScreen;

        // comment interaction state
        int activeCommentId;
        bool resizingComment;
        ImVec2 dragOffset;
        int editingCommentId;
        int selectedCommentId;

        void addNode(const std::string& name, float x, float y);
        void addComment(const ImVec2& posGrid);
        void drawNodes();
        void drawComments();
        void handleContextMenu();
};


#endif //GLITTER_NODEGRAPH_HPP