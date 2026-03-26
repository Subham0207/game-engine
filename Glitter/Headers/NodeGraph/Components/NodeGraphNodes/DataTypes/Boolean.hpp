//
// Created by subha on 26-03-2026.
//

#ifndef GLITTER_BOOLEAN_HPP
#define GLITTER_BOOLEAN_HPP
#include "NodeGraph/Components/Attribute.hpp"
#include "NodeGraph/Components/NodeGraphNode.hpp"

namespace NodeGraphComponents::Node
{
    class Boolean: public NodeGraphNode
    {
    public:
        template<typename... Args>
        explicit Boolean(int& nextOutputPinId, Args&&... args) :
        NodeGraphNode(std::forward<Args>(args)...),
        value(nextOutputPinId++, "Value", TYPE::FIELD)
        {
            outputs().push_back(value);
        }
    private:
        Attribute value;
    };
}
#endif //GLITTER_BOOLEAN_HPP