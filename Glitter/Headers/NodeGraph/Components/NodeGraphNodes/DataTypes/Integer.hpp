//
// Created by subha on 25-03-2026.
//

#ifndef GLITTER_INTEGER_HPP
#define GLITTER_INTEGER_HPP
#include "NodeGraph/Components/NodeGraphNode.hpp"

namespace NodeGraphComponents::Node
{
    class Integer: public NodeGraphNode
    {
    public:
        template<typename... Args>
        explicit Integer(int& nextOutputPinId, Args&&... args) : NodeGraphNode(std::forward<Args>(args)...),
                                           value(nextOutputPinId++, "Value", TYPE::FIELD)
        {
            outputs().push_back(value);
        }
    private:
        Attribute value;
    };
}
#endif //GLITTER_INTEGER_HPP
