//
// Created by subha on 15-03-2026.
//

#ifndef GLITTER_NODEGRAPH_HPP
#define GLITTER_NODEGRAPH_HPP

#include <vector>
#include <string>
#include <imgui.h>

struct Node
{
    int id;
    std::string name;
    float x, y;
    bool positionSet;  // Track if position has been initialized
};

struct CommentBox
{
    int id;
    ImVec2 posGrid{0.0f, 0.0f};     // top-left in grid space (pans with editor)
    ImVec2 size{220.0f, 140.0f};    // in pixels
    static constexpr size_t kMaxText = 1024;
    char text[kMaxText] = "Comment";
};

class NodeGraph
{
    public:
        NodeGraph();
        void drawUI();

    private:
        std::vector<Node> nodes;
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

        void addNode(const std::string& name, float x, float y);
        void addComment(const ImVec2& posGrid);
        void drawNodes();
        void drawComments();
        void handleContextMenu();
};


#endif //GLITTER_NODEGRAPH_HPP