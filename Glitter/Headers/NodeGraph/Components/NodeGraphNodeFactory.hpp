//
// Created by subha on 01-05-2026.
//

#ifndef GLITTER_NODEGRAPHNODEFACTORY_HPP
#define GLITTER_NODEGRAPHNODEFACTORY_HPP

#include <vector>
#include<memory>
#include<imgui.h>
#include "NodeGraphNodes/NodeTypes.hpp"
#include "NodeGraphIdAllocator.hpp"
#include <string>

class NodeGraphNode;
namespace NodeGraphComponents
{
    class NodeGraphNodeFactory
    {
    private:
        NodeGraphIdAllocator* _nodeGraphIdAllocator;
    public:
        explicit NodeGraphNodeFactory(NodeGraphIdAllocator* nodeGraphIdAllocator)
        {
            _nodeGraphIdAllocator = nodeGraphIdAllocator;
        }
        NodeGraphNode* addNode(std::vector<std::unique_ptr<NodeGraphNode>>& nodes, NodeTypes type, const ImVec2& spawnPosScreen = {0.0f,0.0f});
        NodeGraphNode* addFunctionNode(
            std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
            const std::vector<std::string>& args,
            ImVec2 spawnPosScreen = {0.0f,0.0f}
        );
    };
} // NodeGraphComponents

#endif //GLITTER_NODEGRAPHNODEFACTORY_HPP
