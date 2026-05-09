//
// Created by subha on 26-03-2026.
//

#ifndef GLITTER_FUNCTION_HPP
#define GLITTER_FUNCTION_HPP
#include <vector>

#include "NodeGraph/Components/NodeGraphNode.hpp"

namespace NodeGraphComponents::Node::Keywords
{
    class Function: public NodeGraphNode
    {
    public:
        template<typename... Args>
        explicit Function(int& nextOutputPinId,
                          const std::vector<std::string>& argumentNames,
                          Args&&... args):
        NodeGraphNode(std::forward<Args>(args)...)
        {
            for (size_t i = 0; i < argumentNames.size(); ++i)
            {
                std::string argName = argumentNames[i].empty()
                    ? ("arg" + std::to_string(i))
                    : argumentNames[i];
                outputs().emplace_back(nextOutputPinId++, argName, TYPE::PIN);
            }
            setupExecOutput(nextOutputPinId);
        }
        NodeTypes type() override { return NodeTypes::Function; }

        std::string functionName;
    };
}
#endif //GLITTER_FUNCTION_HPP
