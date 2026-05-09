//
// Created by subha on 09-05-2026.
//

#ifndef GLITTER_VARIABLEDECLARATION_HPP
#define GLITTER_VARIABLEDECLARATION_HPP

#include "NodeGraph/Components/Attribute.hpp"
#include "NodeGraph/Components/NodeGraphNode.hpp"

namespace NodeGraphComponents::Node::Variables
{
    class VariableDeclaration : public NodeGraphNode
    {
    public:
        template<typename... Args>
        explicit VariableDeclaration(int& nextInputPinId, int& nextOutputPinId, Args&&... args)
            : NodeGraphNode(std::forward<Args>(args)...), valueInput(nextInputPinId++, "Value", TYPE::PIN)
        {
            inputs().push_back(valueInput);
            setupExecInput(nextInputPinId);
            setupExecOutput(nextOutputPinId);
        }

        NodeTypes type() override { return NodeTypes::VariableDeclaration; }

        std::string variableName;
        std::string declaredType;
        std::string value;

    private:
        Attribute valueInput;
    };
}

#endif //GLITTER_VARIABLEDECLARATION_HPP

