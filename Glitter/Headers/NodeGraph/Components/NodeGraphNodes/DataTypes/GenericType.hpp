//
// Created by subha on 28-03-2026.
//

#ifndef GLITTER_GENERICTYPE_HPP
#define GLITTER_GENERICTYPE_HPP
#include <vector>

#include "Helpers/NodeGraphHelpers.hpp"
#include "NodeGraph/Components/NodeGraphNode.hpp"

namespace NodeGraphComponents::Node
{
    class GenericType: public NodeGraphNode
    {
    public:
        template<typename T, typename... Args>
        explicit GenericType(int& nextOutputPinId, T t, Args&&... args) :
        NodeGraphNode(std::forward<Args>(args)...)
        {
            //Mapping Type T obj to be NodeGraphNode
            auto memberVariableTypes = NodeGraphHelpers::get_field_type_names<T>();
            auto memberVariableNames = NodeGraphHelpers::get_field_names<T>();
            for (std::size_t i = 0; i < memberVariableTypes.size(); ++i)
            {
                auto memberVariableType = memberVariableTypes[i];
                //We only support these two type for now.
                if (memberVariableType != "int" || memberVariableType != "bool")
                    return;

                auto memberVariableName = memberVariableNames[i];
                auto member = Attribute(nextOutputPinId++, memberVariableName, TYPE::FIELD);
                outputs().push_back(member);
            }
            //------------------------------------------

        }
    };
}
#endif //GLITTER_GENERICTYPE_HPP
