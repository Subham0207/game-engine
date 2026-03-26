//
// Created by subha on 26-03-2026.
//

#ifndef GLITTER_RETURN_HPP
#define GLITTER_RETURN_HPP
#include "NodeGraph/Components/NodeGraphNode.hpp"

namespace NodeGraphComponents::Node::Keywords
{
    class Return: public NodeGraphNode
    {
    public:
        template<typename... Args>
        explicit Return(int& nextInputPinId, Args&&... args):
        NodeGraphNode(std::forward<Args>(args)...),
        returnInputPin(nextInputPinId++,"ReturnInputPin", TYPE::PIN)
        {
            inputs().push_back(returnInputPin);

            setupExecInput(nextInputPinId);
        }
    private:
        Attribute returnInputPin;
    };
}
#endif //GLITTER_RETURN_HPP
