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

class NodeGraph
{
    public:
        NodeGraph();
        void drawUI();

    private:
        std::vector<Node> nodes;
        int nextNodeId;
        bool showContextMenu;
        float contextMenuX;
        float contextMenuY;
        // Screen space spawn position (captured on right-click), used to place new nodes.
        ImVec2 spawnPosScreen;

        void addNode(const std::string& name, float x, float y);
        void drawNodes();
        void handleContextMenu();
};


#endif //GLITTER_NODEGRAPH_HPP