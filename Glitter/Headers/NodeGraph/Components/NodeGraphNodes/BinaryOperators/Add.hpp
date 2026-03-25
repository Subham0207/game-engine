//
// Created by subha on 25-03-2026.
//

#ifndef GLITTER_ADD_HPP
#define GLITTER_ADD_HPP
#include "NodeGraph/Components/NodeGraphNode.hpp"
#include <vector>
#include "NodeGraph/Components/Attribute.hpp"

namespace NodeGraphComponents::Node
{
    class Add: public NodeGraphNode
    {
    public:
        template<typename... Args>
        explicit Add(Args&&... args) :
            NodeGraphNode(std::forward<Args>(args)...), A("A", TYPE::PIN), B("B", TYPE::PIN),
            result("Result", TYPE::PIN)
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

#endif //GLITTER_ADD_HPP