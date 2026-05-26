#include <gtest/gtest.h>

#include <imgui.h>
#include <imnodes.h>

#include "NodeGraph/NodeGraph.hpp"
#include "NodeGraph/Components/NodeGraphNodes/DataTypes/Integer.hpp"
#include "NodeGraph/Components/NodeGraphNodes/Keywords/Return.hpp"

TEST(NodeGraphImNodesScope, shouldDrawUIEmbeddedInsideValidFrameWithoutAssertion)
{
    ImGui::CreateContext();
    ImNodes::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1280.0f, 720.0f);
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char* fontPixels = nullptr;
    int fontWidth = 0;
    int fontHeight = 0;
    io.Fonts->GetTexDataAsRGBA32(&fontPixels, &fontWidth, &fontHeight);

    {
        NodeGraph graph;

        ImGui::NewFrame();
        ImGui::Begin("NodeGraphImNodesScopeTestWindow");
        EXPECT_NO_FATAL_FAILURE(graph.drawUIEmbedded());
        ImGui::End();
        ImGui::Render();
    }

    ImNodes::DestroyContext();
    ImGui::DestroyContext();
}

TEST(NodeGraphDeleteNodes, shouldDeleteRegularNodeAndItsConnectedLinksById)
{
    NodeGraph graph;
    auto& ctx = graph.getRenderContext();

    int nextOutputPinId = 4000;
    int nextInputPinId = 3000;

    auto integerNode = std::make_unique<NodeGraphComponents::Node::Integer>(nextOutputPinId, 100, "Integer");
    auto returnNode = std::make_unique<NodeGraphComponents::Node::Keywords::Return>(nextInputPinId, 101, "Return");

    const int integerOutputId = integerNode->outputs().front().getId();
    const int returnInputId = returnNode->inputs().front().getId();

    ctx.nodes.push_back(std::move(integerNode));
    ctx.nodes.push_back(std::move(returnNode));
    ctx.nodeGraphLinks.emplace_back(2000, integerOutputId, returnInputId);

    graph.deleteNodesByIds({100});

    ASSERT_EQ(ctx.nodes.size(), 1u);
    EXPECT_EQ(ctx.nodes.front()->id(), 101);
    EXPECT_TRUE(ctx.nodeGraphLinks.empty());
}

TEST(NodeGraphDeleteNodes, shouldDeleteStateMachineNodeAndAttachedTransitionsById)
{
    NodeGraph graph;
    auto& ctx = graph.getRenderContext();

    StateMachineNode from;
    from.id = 9000;
    StateMachineNode to;
    to.id = 9001;

    ctx.stateNodes.push_back(from);
    ctx.stateNodes.push_back(to);

    StateMachineLink transition;
    transition.id = 500;
    transition.fromNodeId = from.id;
    transition.toNodeId = to.id;
    ctx.stateLinks.push_back(transition);

    graph.deleteNodesByIds({9000});

    ASSERT_EQ(ctx.stateNodes.size(), 1u);
    EXPECT_EQ(ctx.stateNodes.front().id, 9001);
    EXPECT_TRUE(ctx.stateLinks.empty());
}

