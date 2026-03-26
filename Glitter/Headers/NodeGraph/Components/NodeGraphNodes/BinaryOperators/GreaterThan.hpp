//
// Created by subha on 25-03-2026.
//

#ifndef GLITTER_GREATERTHAN_HPP
#define GLITTER_GREATERTHAN_HPP
#include "NodeGraph/Components/NodeGraphNode.hpp"

namespace NodeGraphComponents::Node
{
    class GreaterThan: public NodeGraphNode
    {
    public:
        template<typename ...Args>
        explicit GreaterThan(
            int& nextInputPinId,
            int& nextOutputPinId,
            Args&&... args
            ):
        NodeGraphNode(std::forward<Args>(args)...),
        A(nextInputPinId++,"A", TYPE::PIN),
        B(nextInputPinId++,"B", TYPE::PIN),
        isAGreaterThanB(nextOutputPinId++,"isAGreaterThanB", TYPE::PIN)
        {
            inputs().push_back(A);
            inputs().push_back(B);

            outputs().push_back(isAGreaterThanB);
        }

    private:
        Attribute A;
        Attribute B;

        Attribute isAGreaterThanB;
    };
}
#endif //GLITTER_GREATERTHAN_HPP
