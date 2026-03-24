//
// Created by subha on 24-03-2026.
//

#ifndef GLITTER_STATEMACHINEGRAPH_HPP
#define GLITTER_STATEMACHINEGRAPH_HPP

#include "NodeGraph.hpp"
#include "StateMachineJsonExporter.hpp"

// A specialized NodeGraph focused on state-machine authoring.
//
// Today NodeGraph registers multiple views (generic nodes/comments + state-machine).
// This derived type exists as a clear entry point for tools that only need the
// state-machine editor and related export utilities.
class StateMachineGraph final : public NodeGraph
{
public:
    StateMachineGraph()
    {
        // Add state-machine specific UI via the generic NodeGraph screen-space hook.
        setScreenSpaceUi(
            [](NodeGraphRenderContext& ctx, void*) {
                // Find active node (if any)
                int activeId = -1;
                for (const auto& n : ctx.stateNodes)
                {
                    if (n.active)
                    {
                        activeId = n.id;
                        break;
                    }
                }

                // Simple toolbar button in screen space.
                ImGui::SetCursorPos(ImVec2(8.0f, 8.0f));
                ImGui::BeginGroup();

                const bool hasActive = (activeId != -1);
                if (!hasActive)
                    ImGui::BeginDisabled();

                if (ImGui::SmallButton("Copy SM JSON"))
                {
                    const std::string json = StateMachineJsonExporter::ExportChainJson(
                        ctx.stateNodes, ctx.stateLinks, activeId);
                    ImGui::SetClipboardText(json.c_str());
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                    ImGui::SetTooltip("Copies JSON for the chain reachable from the active state");

                if (!hasActive)
                    ImGui::EndDisabled();

                ImGui::EndGroup();
            },
            nullptr);
    }
    ~StateMachineGraph() override = default;
};

#endif // GLITTER_STATEMACHINEGRAPH_HPP


