//
// Created by subha on 28-03-2026.
//

#ifndef GLITTER_GENERICTYPE_HPP
#define GLITTER_GENERICTYPE_HPP
#include <vector>
#include <string>

#include "Helpers/NodeGraphHelpers.hpp"
#include "NodeGraph/Components/NodeGraphNode.hpp"

namespace NodeGraphComponents::Node
{
    struct GenericMemberSpec
    {
        std::string name;
        std::string literalValue;
        bool isBoolean = false;
    };

    class GenericType: public NodeGraphNode
    {
    public:
        // Runtime-driven constructor used by Lua decompiler.
        template<typename... Args>
        explicit GenericType(int& nextOutputPinId,
                             const std::vector<GenericMemberSpec>& members,
                             Args&&... args) :
        NodeGraphNode(std::forward<Args>(args)...)
        {
            for (const auto& memberSpec : members)
            {
                auto member = Attribute(nextOutputPinId++, memberSpec.name, TYPE::FIELD);
                member.setValue(memberSpec.literalValue);
                outputs().push_back(member);
            }
        }

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
                if (memberVariableType != "int" && memberVariableType != "bool")
                    continue;

                auto memberVariableName = memberVariableNames[i];
                auto member = Attribute(nextOutputPinId++, memberVariableName, TYPE::FIELD);
                outputs().push_back(member);
            }
            //------------------------------------------

        }
    };
}
#endif //GLITTER_GENERICTYPE_HPP
