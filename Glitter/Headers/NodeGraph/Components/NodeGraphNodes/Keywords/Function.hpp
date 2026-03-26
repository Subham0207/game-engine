//
// Created by subha on 26-03-2026.
//

#ifndef GLITTER_FUNCTION_HPP
#define GLITTER_FUNCTION_HPP
#include "NodeGraph/Components/NodeGraphNode.hpp"

namespace NodeGraphComponents::Node::Keywords
{
    class Function: public NodeGraphNode
    {
    public:
        template<typename... Args>
        explicit Function(int& nextOutputPinId, Args&&... args):
        NodeGraphNode(std::forward<Args>(args)...),
        startPin(nextOutputPinId++,"StartPin", TYPE::PIN)
        {
            outputs().push_back(startPin);
        }
    private:
        Attribute startPin;
    };
}
#endif //GLITTER_FUNCTION_HPP
