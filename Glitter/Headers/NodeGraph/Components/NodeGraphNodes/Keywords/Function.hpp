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
        explicit Function(int& nextOutputPinId, const int argumentsSize, Args&&... args):
        NodeGraphNode(std::forward<Args>(args)...)
        {
            FunctionArgs.resize(argumentsSize);
            for (int i = 0; i < FunctionArgs.size(); ++i)
            {
                FunctionArgs[i] = Attribute(nextOutputPinId++,"ArgPin", TYPE::FIELD);
            }
            setupExecOutput(nextOutputPinId);
        }
    private:
        std::vector<Attribute> FunctionArgs;
    };
}
#endif //GLITTER_FUNCTION_HPP
