//
// Created by subha on 09-05-2026.
//

#ifndef GLITTER_LESSTHAN_HPP
#define GLITTER_LESSTHAN_HPP

#include "NodeGraph/Components/NodeGraphNode.hpp"
#include "NodeGraph/Components/Attribute.hpp"

namespace NodeGraphComponents::Node
{
    class LessThan: public NodeGraphNode
    {
    public:
        template<typename... Args>
        explicit LessThan(int& nextInputPinId, int& nextOutputPinId, Args&&... args)
            : NodeGraphNode(std::forward<Args>(args)...),
              A(nextInputPinId++, "A", TYPE::PIN),
              B(nextInputPinId++, "B", TYPE::PIN),
              result(nextOutputPinId++, "Result", TYPE::PIN)
        {
            inputs().push_back(A);
            inputs().push_back(B);

            outputs().push_back(result);

            setupExecInput(nextInputPinId);
            setupExecOutput(nextOutputPinId);
        }

        NodeTypes type() override { return NodeTypes::LessThan; }

    private:
        Attribute A;
        Attribute B;
        Attribute result;
    };
}

#endif //GLITTER_LESSTHAN_HPP

