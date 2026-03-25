//
// Created by subha on 25-03-2026.
//

#ifndef GLITTER_SUBTRACT_HPP
#define GLITTER_SUBTRACT_HPP
#include "NodeGraph/Components/NodeGraphNode.hpp"

namespace NodeGraphComponents::Node
{
    class Subtract: public NodeGraphNode
    {
    public:
        template<typename... Args>
        explicit Subtract(int& nextInputPinId, int& nextOutputPinid, Args&&... args) :
            NodeGraphNode(std::forward<Args>(args)...),
            A(nextInputPinId++,"A", TYPE::PIN),
            B(nextInputPinId++,"B", TYPE::PIN),
            result(nextOutputPinid++,"Result", TYPE::PIN)
        {
            inputs().push_back(A);
            inputs().push_back(B);

            outputs().push_back(result);
        }

    private:
        Attribute A;
        Attribute B;

        Attribute result;
    };
}
#endif //GLITTER_SUBTRACT_HPP
