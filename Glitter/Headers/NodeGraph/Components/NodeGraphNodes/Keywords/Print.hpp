//
// Created by subha on 26-03-2026.
//

#ifndef GLITTER_PRINT_HPP
#define GLITTER_PRINT_HPP
#include "NodeGraph/Components/NodeGraphNode.hpp"

namespace NodeGraphComponents::Node::Keywords
{
    class Print: public NodeGraphNode
    {
    public:
        template<typename... Args>
        explicit Print(int& nextInputPinId, Args&&... args):
        NodeGraphNode(std::forward<Args>(args)...),
        PrintInputPin(nextInputPinId++,"PrintInputPin", TYPE::PIN)
        {
            inputs().push_back(PrintInputPin);

            setupExecInput(nextInputPinId);
        }
    private:
        Attribute PrintInputPin;
    };
}
#endif //GLITTER_PRINT_HPP