//
// Created by subha on 03-05-2026.
//

#ifndef GLITTER_NODEGRAPHIDALLOCATOR_HPP
#define GLITTER_NODEGRAPHIDALLOCATOR_HPP
#include "NodeGraph/NodeGraphIdRanges.hpp"

namespace NodeGraphComponents
{
    class NodeGraphIdAllocator
    {
    public:
        int nextNodeId = NodeGraphIdBase(NodeGraphElementIdBase::NodeGraphNode);
        int nextLinkId = NodeGraphIdBase(NodeGraphElementIdBase::NodeGraphNodeLink);
        int nextInputPinId = NodeGraphIdBase(NodeGraphElementIdBase::NodeGraphNodeInputAttribute);
        int nextOutputPinId = NodeGraphIdBase(NodeGraphElementIdBase::NodeGraphNodeOutputAttribute);
    };
}


#endif //GLITTER_NODEGRAPHIDALLOCATOR_HPP
