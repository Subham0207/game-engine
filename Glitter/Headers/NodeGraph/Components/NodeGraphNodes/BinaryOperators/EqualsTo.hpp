//
// Created by subha on 26-03-2026.
//

#ifndef GLITTER_EQUALSTO_HPP
#define GLITTER_EQUALSTO_HPP
#include "NodeGraph/Components/NodeGraphNode.hpp"

namespace NodeGraphComponents::Node
{
    /*
     * input { firstValue, secondValue }
     * return (firstValue == secondValue)
     */
    class EqualsTo: public NodeGraphNode
    {
    public:
        template<typename... Args>
        explicit EqualsTo(int& nextInputPinId, int& nextOutputPinId, Args&&... args) :
        NodeGraphNode(std::forward<Args>(args)...),
        FirstValue(nextInputPinId++, "FirstValue", TYPE::PIN),
        SecondValue(nextInputPinId++, "SecondValue", TYPE::PIN),
        Result(nextOutputPinId++, "==", TYPE::PIN)
        {
            inputs().push_back(FirstValue);
            inputs().push_back(SecondValue);

            outputs().push_back(Result);
        }

        NodeTypes type() override { return NodeTypes::EqualsTo; }
    private:
        Attribute FirstValue;
        Attribute SecondValue;

        Attribute Result;
    };
}
#endif //GLITTER_EQUALSTO_HPP