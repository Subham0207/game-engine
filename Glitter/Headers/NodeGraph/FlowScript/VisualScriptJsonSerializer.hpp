#pragma once

#include <boost/json.hpp>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Add.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Divide.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/EqualsTo.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/GreaterThan.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/LessThan.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Modulo.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Multiply.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/NotEqualsTo.hpp"
#include "NodeGraph/Components/NodeGraphNodes/BinaryOperators/Subtract.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Boolean.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/GenericType.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Integer.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Function.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Print.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Return.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Variables/GetVariable.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Variables/VariableDeclaration.hpp"
#include "NodeGraph/Components/NodeGraphNode.hpp"
#include "NodeGraph/Components/NodeGraphNodeLink.hpp"
#include "NodeGraph/NodeGraphIdRanges.hpp"

namespace Flowscript::Serialization
{
    namespace json = boost::json;

    class VisualScriptJsonSerializer
    {
    public:
        static inline bool SerializeToFile(
            const std::filesystem::path& filePath,
            const std::vector<std::unique_ptr<NodeGraphNode>>& nodes,
            const std::vector<NodeGraphNodeLink>& links,
            std::string* outError = nullptr)
        {
            try
            {
                json::object root;
                root["formatVersion"] = 1;

                json::array nodesJson;
                nodesJson.reserve(nodes.size());

                for (const auto& node : nodes)
                {
                    if (!node)
                        continue;

                    json::object nodeObj;
                    nodeObj["id"] = node->id();
                    nodeObj["type"] = static_cast<int>(node->type());
                    nodeObj["name"] = node->name();

                    const ImVec2 pos = node->spawnPosScreen();
                    nodeObj["x"] = pos.x;
                    nodeObj["y"] = pos.y;

                    json::array inputsJson;
                    inputsJson.reserve(node->inputs().size());
                    for (const auto& input : node->inputs())
                        inputsJson.push_back(toJsonAttribute(input));
                    nodeObj["inputs"] = std::move(inputsJson);

                    json::array outputsJson;
                    outputsJson.reserve(node->outputs().size());
                    for (const auto& output : node->outputs())
                        outputsJson.push_back(toJsonAttribute(output));
                    nodeObj["outputs"] = std::move(outputsJson);

                    if (node->hasExecInput())
                        nodeObj["execInput"] = toJsonAttribute(*node->getExecInput());
                    if (node->hasExecOutput())
                        nodeObj["execOutput"] = toJsonAttribute(*node->getExecOutput());

                    json::object metadataObj;
                    if (const auto* functionNode = dynamic_cast<const NodeGraphComponents::Node::Keywords::Function*>(node.get()))
                        metadataObj["functionName"] = functionNode->functionName;
                    if (const auto* variableDeclNode = dynamic_cast<const NodeGraphComponents::Node::Variables::VariableDeclaration*>(node.get()))
                    {
                        metadataObj["variableName"] = variableDeclNode->variableName;
                        metadataObj["declaredType"] = variableDeclNode->declaredType;
                        metadataObj["value"] = variableDeclNode->value;
                    }
                    if (const auto* getVariableNode = dynamic_cast<const NodeGraphComponents::Node::Variables::GetVariable*>(node.get()))
                        metadataObj["variableName"] = getVariableNode->variableName;

                    if (!metadataObj.empty())
                        nodeObj["metadata"] = std::move(metadataObj);

                    nodesJson.push_back(std::move(nodeObj));
                }
                root["nodes"] = std::move(nodesJson);

                json::array linksJson;
                linksJson.reserve(links.size());
                for (const auto& link : links)
                {
                    json::object linkObj;
                    linkObj["id"] = link.id();
                    linkObj["startAttr"] = link.startAttr();
                    linkObj["endAttr"] = link.endAttr();
                    linksJson.push_back(std::move(linkObj));
                }
                root["links"] = std::move(linksJson);

                std::error_code ec;
                std::filesystem::create_directories(filePath.parent_path(), ec);

                std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
                if (!file.is_open())
                {
                    if (outError)
                        *outError = "Failed to open file for writing: " + filePath.string();
                    return false;
                }

                file << json::serialize(root);
                return true;
            }
            catch (const std::exception& ex)
            {
                if (outError)
                    *outError = ex.what();
                return false;
            }
        }

        static inline bool DeserializeFromFile(
            const std::filesystem::path& filePath,
            std::vector<std::unique_ptr<NodeGraphNode>>& outNodes,
            std::vector<NodeGraphNodeLink>& outLinks,
            std::string* outError = nullptr)
        {
            try
            {
                std::ifstream file(filePath, std::ios::binary);
                if (!file.is_open())
                {
                    if (outError)
                        *outError = "Failed to open file for reading: " + filePath.string();
                    return false;
                }

                std::stringstream buffer;
                buffer << file.rdbuf();
                const std::string jsonText = buffer.str();
                if (jsonText.empty())
                {
                    if (outError)
                        *outError = "Visual script file is empty: " + filePath.string();
                    return false;
                }

                json::value rootValue = json::parse(jsonText);
                if (!rootValue.is_object())
                {
                    if (outError)
                        *outError = "Visual script root must be a JSON object.";
                    return false;
                }

                const auto& root = rootValue.as_object();
                const auto nodesIt = root.find("nodes");
                const auto linksIt = root.find("links");
                if (nodesIt == root.end() || !nodesIt->value().is_array() || linksIt == root.end() || !linksIt->value().is_array())
                {
                    if (outError)
                        *outError = "Visual script JSON is missing required 'nodes' or 'links' arrays.";
                    return false;
                }

                std::vector<std::unique_ptr<NodeGraphNode>> loadedNodes;
                std::vector<NodeGraphNodeLink> loadedLinks;

                for (const auto& nodeValue : nodesIt->value().as_array())
                {
                    if (!nodeValue.is_object())
                        continue;

                    auto node = createNodeFromJson(nodeValue.as_object());
                    if (!node)
                        continue;
                    loadedNodes.push_back(std::move(node));
                }

                for (const auto& linkValue : linksIt->value().as_array())
                {
                    if (!linkValue.is_object())
                        continue;
                    const auto& obj = linkValue.as_object();
                    loadedLinks.emplace_back(
                        json::value_to<int>(obj.at("id")),
                        json::value_to<int>(obj.at("startAttr")),
                        json::value_to<int>(obj.at("endAttr"))
                    );
                }

                outNodes = std::move(loadedNodes);
                outLinks = std::move(loadedLinks);
                return true;
            }
            catch (const std::exception& ex)
            {
                if (outError)
                    *outError = ex.what();
                return false;
            }
        }

    private:
        static inline json::object toJsonAttribute(const NodeAttribute& attribute)
        {
            json::object out;
            out["id"] = attribute.getId();
            out["name"] = attribute.getName();
            out["type"] = static_cast<int>(attribute.getType());
            if (attribute.getType() == NodeAttributeType::FIELD)
                out["value"] = std::string(attribute.getValueBuff());
            return out;
        }

        static inline int attributeIdSeed(const json::array& attributes, const json::object& nodeObj, const char* execKey, const int defaultSeed)
        {
            int seed = defaultSeed;
            bool found = false;

            for (const auto& value : attributes)
            {
                if (!value.is_object())
                    continue;
                const auto& attrObj = value.as_object();
                const auto idIt = attrObj.find("id");
                if (idIt == attrObj.end())
                    continue;

                const int attrId = json::value_to<int>(idIt->value());
                if (!found || attrId < seed)
                {
                    seed = attrId;
                    found = true;
                }
            }

            const auto execIt = nodeObj.find(execKey);
            if (execIt != nodeObj.end() && execIt->value().is_object())
            {
                const auto& execObj = execIt->value().as_object();
                const auto idIt = execObj.find("id");
                if (idIt != execObj.end())
                {
                    const int execId = json::value_to<int>(idIt->value());
                    if (!found || execId < seed)
                        seed = execId;
                }
            }

            return seed;
        }

        static inline std::unique_ptr<NodeGraphNode> createNodeFromJson(const json::object& nodeObj)
        {
            const int id = json::value_to<int>(nodeObj.at("id"));
            const auto type = static_cast<NodeTypes>(json::value_to<int>(nodeObj.at("type")));
            const std::string name = json::value_to<std::string>(nodeObj.at("name"));
            const float x = json::value_to<float>(nodeObj.at("x"));
            const float y = json::value_to<float>(nodeObj.at("y"));

            const json::array emptyArray;
            const json::array& inputAttributes = nodeObj.if_contains("inputs") && nodeObj.at("inputs").is_array()
                ? nodeObj.at("inputs").as_array()
                : emptyArray;
            const json::array& outputAttributes = nodeObj.if_contains("outputs") && nodeObj.at("outputs").is_array()
                ? nodeObj.at("outputs").as_array()
                : emptyArray;

            int nextInputPinId = attributeIdSeed(inputAttributes, nodeObj, "execInput", NodeGraphIdBase(NodeGraphElementIdBase::NodeGraphNodeInputAttribute));
            int nextOutputPinId = attributeIdSeed(outputAttributes, nodeObj, "execOutput", NodeGraphIdBase(NodeGraphElementIdBase::NodeGraphNodeOutputAttribute));

            std::unique_ptr<NodeGraphNode> node;
            switch (type)
            {
                case NodeTypes::Add:
                    node = std::make_unique<NodeGraphComponents::Node::Add>(nextInputPinId, nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::Subtract:
                    node = std::make_unique<NodeGraphComponents::Node::Subtract>(nextInputPinId, nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::Multiply:
                    node = std::make_unique<NodeGraphComponents::Node::Multiply>(nextInputPinId, nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::Divide:
                    node = std::make_unique<NodeGraphComponents::Node::Divide>(nextInputPinId, nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::Modulo:
                    node = std::make_unique<NodeGraphComponents::Node::Modulo>(nextInputPinId, nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::LessThan:
                    node = std::make_unique<NodeGraphComponents::Node::LessThan>(nextInputPinId, nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::GreaterThan:
                    node = std::make_unique<NodeGraphComponents::Node::GreaterThan>(nextInputPinId, nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::EqualsTo:
                    node = std::make_unique<NodeGraphComponents::Node::EqualsTo>(nextInputPinId, nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::NotEqualsTo:
                    node = std::make_unique<NodeGraphComponents::Node::NotEqualsTo>(nextInputPinId, nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::Integer:
                    node = std::make_unique<NodeGraphComponents::Node::Integer>(nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::Boolean:
                    node = std::make_unique<NodeGraphComponents::Node::Boolean>(nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::Function:
                {
                    std::vector<std::string> argumentNames;
                    argumentNames.reserve(outputAttributes.size());
                    for (const auto& outputValue : outputAttributes)
                    {
                        if (!outputValue.is_object())
                            continue;
                        const auto& outputObj = outputValue.as_object();
                        const auto nameIt = outputObj.find("name");
                        if (nameIt != outputObj.end() && nameIt->value().is_string())
                            argumentNames.push_back(json::value_to<std::string>(nameIt->value()));
                    }

                    node = std::make_unique<NodeGraphComponents::Node::Keywords::Function>(nextOutputPinId, argumentNames, id, name, x, y);
                    break;
                }
                case NodeTypes::Print:
                    node = std::make_unique<NodeGraphComponents::Node::Keywords::Print>(nextInputPinId, nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::Return:
                    node = std::make_unique<NodeGraphComponents::Node::Keywords::Return>(nextInputPinId, id, name, x, y);
                    break;
                case NodeTypes::VariableDeclaration:
                    node = std::make_unique<NodeGraphComponents::Node::Variables::VariableDeclaration>(nextInputPinId, nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::GetVariable:
                    node = std::make_unique<NodeGraphComponents::Node::Variables::GetVariable>(nextOutputPinId, id, name, x, y);
                    break;
                case NodeTypes::Generic:
                {
                    std::vector<NodeGraphComponents::Node::GenericMemberSpec> members;
                    members.reserve(outputAttributes.size());

                    for (const auto& outputValue : outputAttributes)
                    {
                        if (!outputValue.is_object())
                            continue;
                        const auto& outputObj = outputValue.as_object();

                        NodeGraphComponents::Node::GenericMemberSpec member;
                        member.name = json::value_to<std::string>(outputObj.at("name"));
                        if (const auto valueIt = outputObj.find("value"); valueIt != outputObj.end() && valueIt->value().is_string())
                        {
                            member.literalValue = json::value_to<std::string>(valueIt->value());
                            member.isBoolean = member.literalValue == "true" || member.literalValue == "false";
                        }
                        members.push_back(std::move(member));
                    }

                    const bool asDestructuringNode = !inputAttributes.empty();
                    node = std::make_unique<NodeGraphComponents::Node::GenericType>(nextInputPinId, nextOutputPinId, members, asDestructuringNode, id, name, x, y);
                    break;
                }
            }

            if (!node)
                return nullptr;

            if (auto* functionNode = dynamic_cast<NodeGraphComponents::Node::Keywords::Function*>(node.get()))
            {
                if (const auto* metadata = nodeObj.if_contains("metadata"); metadata && metadata->is_object())
                {
                    const auto& metadataObj = metadata->as_object();
                    if (const auto it = metadataObj.find("functionName"); it != metadataObj.end() && it->value().is_string())
                        functionNode->functionName = json::value_to<std::string>(it->value());
                }
            }

            if (auto* variableDeclNode = dynamic_cast<NodeGraphComponents::Node::Variables::VariableDeclaration*>(node.get()))
            {
                if (const auto* metadata = nodeObj.if_contains("metadata"); metadata && metadata->is_object())
                {
                    const auto& metadataObj = metadata->as_object();
                    if (const auto it = metadataObj.find("variableName"); it != metadataObj.end() && it->value().is_string())
                        variableDeclNode->variableName = json::value_to<std::string>(it->value());
                    if (const auto it = metadataObj.find("declaredType"); it != metadataObj.end() && it->value().is_string())
                        variableDeclNode->declaredType = json::value_to<std::string>(it->value());
                    if (const auto it = metadataObj.find("value"); it != metadataObj.end() && it->value().is_string())
                        variableDeclNode->value = json::value_to<std::string>(it->value());
                }
            }

            if (auto* getVariableNode = dynamic_cast<NodeGraphComponents::Node::Variables::GetVariable*>(node.get()))
            {
                if (const auto* metadata = nodeObj.if_contains("metadata"); metadata && metadata->is_object())
                {
                    const auto& metadataObj = metadata->as_object();
                    if (const auto it = metadataObj.find("variableName"); it != metadataObj.end() && it->value().is_string())
                        getVariableNode->variableName = json::value_to<std::string>(it->value());
                }
            }

            node->setSpawnPosScreen(ImVec2(x, y));
            node->markPositionSet(true);
            return node;
        }
    };
}


