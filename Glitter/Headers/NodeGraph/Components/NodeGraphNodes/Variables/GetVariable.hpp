//
// Created by subha on 09-05-2026.
//
#ifndef GLITTER_GETVARIABLE_HPP
#define GLITTER_GETVARIABLE_HPP
#include "NodeGraph/Components/Attribute.hpp"
#include "NodeGraph/Components/NodeGraphNode.hpp"
namespace NodeGraphComponents::Node::Variables
{
    class GetVariable : public NodeGraphNode
    {
    public:
        template<typename... Args>
        explicit GetVariable(int& nextOutputPinId, Args&&... args)
            : NodeGraphNode(std::forward<Args>(args)...), valueOutput(nextOutputPinId++, "Value", TYPE::PIN)
        {
            outputs().push_back(valueOutput);
        }
        NodeTypes type() override { return NodeTypes::GetVariable; }

        std::string variableName = "var";
    private:
        Attribute valueOutput;
    };
}
#endif //GLITTER_GETVARIABLE_HPP
