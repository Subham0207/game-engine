//
// Created by subha on 15-03-2026.
//

#include "../Headers/NodeGraph/NodeGraph.hpp"
#include <imnodes.h>
#include <imgui.h>

void NodeGraph::drawUI()
{
    ImGui::Begin("node editor");

    ImNodes::BeginNodeEditor();
    ImNodes::EndNodeEditor();

    ImGui::End();
}
