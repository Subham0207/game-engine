#include <gtest/gtest.h>

#include <imgui.h>
#include <imnodes.h>

#include "NodeGraph/NodeGraph.hpp"

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

