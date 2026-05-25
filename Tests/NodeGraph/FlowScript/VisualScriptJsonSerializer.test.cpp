#include <gtest/gtest.h>

#include <filesystem>

#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Integer.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Function.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Variables/VariableDeclaration.hpp"
#include "NodeGraph/FlowScript/VisualScriptJsonSerializer.hpp"

using NodeGraphComponents::Node::Integer;
using NodeGraphComponents::Node::Keywords::Function;
using NodeGraphComponents::Node::Variables::VariableDeclaration;
using Flowscript::Serialization::VisualScriptJsonSerializer;

TEST(VisualScriptJsonSerializer, shouldSerializeAndDeserializeFlowScriptGraph)
{
    int nextInputPinId = 3000;
    int nextOutputPinId = 4000;

    auto functionNode = std::make_unique<Function>(nextOutputPinId, std::vector<std::string>{"t"}, 100, "Function", 10.0f, 20.0f);
    functionNode->functionName = "Tick";
    functionNode->setSpawnPosScreen(ImVec2(10.0f, 20.0f));
    functionNode->markPositionSet(true);

    auto varNode = std::make_unique<VariableDeclaration>(nextInputPinId, nextOutputPinId, 200, "VariableDeclaration", 70.0f, 90.0f);
    varNode->variableName = "speed";
    varNode->declaredType = "Number";
    varNode->value = "5";
    varNode->setSpawnPosScreen(ImVec2(70.0f, 90.0f));
    varNode->markPositionSet(true);

    auto intNode = std::make_unique<Integer>(nextOutputPinId, 300, "Integer", 120.0f, 170.0f);
    intNode->outputs()[0].setValue("42");
    intNode->setSpawnPosScreen(ImVec2(120.0f, 170.0f));
    intNode->markPositionSet(true);

    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    nodes.push_back(std::move(functionNode));
    nodes.push_back(std::move(varNode));
    nodes.push_back(std::move(intNode));

    std::vector<NodeGraphNodeLink> links;
    links.emplace_back(2000, nodes[0]->getExecOutput()->getId(), nodes[1]->getExecInput()->getId());

    const std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "VisualScriptJsonSerializer_shouldSerializeAndDeserializeFlowScriptGraph.flowscript";

    std::string error;
    const bool saveOk = VisualScriptJsonSerializer::SerializeToFile(tempPath, nodes, links, &error);
    ASSERT_TRUE(saveOk) << error;

    std::vector<std::unique_ptr<NodeGraphNode>> loadedNodes;
    std::vector<NodeGraphNodeLink> loadedLinks;
    const bool loadOk = VisualScriptJsonSerializer::DeserializeFromFile(tempPath, loadedNodes, loadedLinks, &error);
    ASSERT_TRUE(loadOk) << error;

    ASSERT_EQ(loadedNodes.size(), 3u);
    ASSERT_EQ(loadedLinks.size(), 1u);

    auto* loadedFunction = dynamic_cast<Function*>(loadedNodes[0].get());
    ASSERT_NE(loadedFunction, nullptr);
    EXPECT_EQ(loadedFunction->functionName, "Tick");
    EXPECT_FLOAT_EQ(loadedFunction->spawnPosScreen().x, 10.0f);
    EXPECT_FLOAT_EQ(loadedFunction->spawnPosScreen().y, 20.0f);

    auto* loadedVar = dynamic_cast<VariableDeclaration*>(loadedNodes[1].get());
    ASSERT_NE(loadedVar, nullptr);
    EXPECT_EQ(loadedVar->variableName, "speed");
    EXPECT_EQ(loadedVar->declaredType, "Number");
    EXPECT_EQ(loadedVar->value, "5");

    std::error_code ec;
    std::filesystem::remove(tempPath, ec);
}

TEST(VisualScriptJsonSerializer, shouldKeepDeserializedNodesPendingFirstPositionApply)
{
    int nextOutputPinId = 4000;

    auto functionNode = std::make_unique<Function>(nextOutputPinId, std::vector<std::string>{"t"}, 100, "Function", 333.0f, 222.0f);
    functionNode->functionName = "condition";
    functionNode->setSpawnPosScreen(ImVec2(333.0f, 222.0f));
    functionNode->markPositionSet(true);

    std::vector<std::unique_ptr<NodeGraphNode>> nodes;
    nodes.push_back(std::move(functionNode));
    std::vector<NodeGraphNodeLink> links;

    const std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "VisualScriptJsonSerializer_shouldKeepDeserializedNodesPendingFirstPositionApply.flowscript";

    std::string error;
    const bool saveOk = VisualScriptJsonSerializer::SerializeToFile(tempPath, nodes, links, &error);
    ASSERT_TRUE(saveOk) << error;

    std::vector<std::unique_ptr<NodeGraphNode>> loadedNodes;
    std::vector<NodeGraphNodeLink> loadedLinks;
    const bool loadOk = VisualScriptJsonSerializer::DeserializeFromFile(tempPath, loadedNodes, loadedLinks, &error);
    ASSERT_TRUE(loadOk) << error;

    ASSERT_EQ(loadedNodes.size(), 1u);
    EXPECT_FALSE(loadedNodes[0]->positionSet());
    EXPECT_TRUE(loadedNodes[0]->hasSpawnPosScreen());
    EXPECT_FLOAT_EQ(loadedNodes[0]->spawnPosScreen().x, 333.0f);
    EXPECT_FLOAT_EQ(loadedNodes[0]->spawnPosScreen().y, 222.0f);

    std::error_code ec;
    std::filesystem::remove(tempPath, ec);
}

