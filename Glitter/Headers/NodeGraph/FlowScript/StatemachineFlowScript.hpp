//
// Created by subha on 26-03-2026.
//

#ifndef GLITTER_STATEMACHINEFLOWSCRIPT_HPP
#define GLITTER_STATEMACHINEFLOWSCRIPT_HPP
#include "FlowScript.hpp"
#include "Helpers/NodeGraphHelpers.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/GenericType.hpp"

#include <boost/pfr.hpp>
#include <type_traits>
#include <filesystem>

class StatemachineFlowScript: public FlowScript
{
public:
    StatemachineFlowScript();
    ~StatemachineFlowScript() override = default;

    void draw();

    void setSelectedLink(StateMachineLink* link);

    template<typename T>
    void setContextObject(const T& context)
    {
        contextMembers.clear();
        const auto fieldNames = NodeGraphHelpers::get_field_names<T>();

        size_t index = 0;
        boost::pfr::for_each_field(context, [&](const auto& field)
        {
            using FieldType = std::decay_t<decltype(field)>;
            if (index < fieldNames.size())
            {
                NodeGraphComponents::Node::GenericMemberSpec spec;
                spec.name = fieldNames[index];

                if constexpr (std::is_same_v<FieldType, bool>)
                {
                    spec.literalValue = "false";
                    spec.isBoolean = true;
                    contextMembers.push_back(std::move(spec));
                }
                else if constexpr (std::is_integral_v<FieldType> || std::is_floating_point_v<FieldType>)
                {
                    spec.literalValue = "0";
                    spec.isBoolean = false;
                    contextMembers.push_back(std::move(spec));
                }
            }
            ++index;
        });
    }

    void close()
    {
        showUI = false;
        selectedLink = nullptr;
    }

    const std::string& compile() override;

    static const std::string& defaultConditionChunk();

private:
    static std::string trimCopy(const std::string& value);
    static std::string unwrapConditionChunk(const std::string& storedCondition);
    static std::string wrapCompiledEditorScript(const std::string& compiledEditorScript);
    void ensureContextNode();
    void ensureContextInputConnection();
    void ensureSelectedLinkScriptPaths() const;
    std::filesystem::path resolveScriptPath(const std::string& storedPath) const;
    static std::string readTextFile(const std::filesystem::path& path);
    static bool writeTextFile(const std::filesystem::path& path, const std::string& content);

    bool showUI;
    StateMachineLink* selectedLink;
    std::vector<NodeGraphComponents::Node::GenericMemberSpec> contextMembers;
};


#endif //GLITTER_STATEMACHINEFLOWSCRIPT_HPP