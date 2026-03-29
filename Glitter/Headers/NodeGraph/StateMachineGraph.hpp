//
// Created by subha on 24-03-2026.
//

#ifndef GLITTER_STATEMACHINEGRAPH_HPP
#define GLITTER_STATEMACHINEGRAPH_HPP

#include "NodeGraph.hpp"
#include "StateMachineJsonExporter.hpp"
#include "Views/StateMachineView.hpp"
#include "Helpers/Shared.hpp"
#include "Helpers/NodeGraphHelpers.hpp"
#include "EngineState.hpp"
#include <filesystem>

// A specialized NodeGraph focused on state-machine authoring.
//
// Today NodeGraph registers multiple views (generic nodes/comments + state-machine).
// This derived type exists as a clear entry point for tools that only need the
// state-machine editor and related export utilities.
class StateMachineGraph final : public NodeGraph
{
public:
    StateMachineGraph(StatemachineFlowScript* ref)
    {
        auto view = views.findView<StateMachineView>();
        view->setFlowScriptRef(ref);

        filename.setText("State Machine Editor");

        // Add state-machine specific UI via the generic NodeGraph screen-space hook.
        setScreenSpaceUi(
            [](NodeGraphRenderContext& ctx, void* user) {
                auto* self = static_cast<StateMachineGraph*>(user);

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
                    self->json = StateMachineJsonExporter::ExportChainJson(
                        ctx.stateNodes, ctx.stateLinks, activeId);
                    ImGui::SetClipboardText(self->json.c_str());
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                    ImGui::SetTooltip("Copies JSON for the chain reachable from the active state");

                if (ImGui::SmallButton("Save"))
                {
                    self->json = StateMachineJsonExporter::ExportChainJson(
                        ctx.stateNodes, ctx.stateLinks, activeId);
                    self->save();
                }

                if (!hasActive)
                    ImGui::EndDisabled();

                ImGui::EndGroup();
            },
            this);
    }
    ~StateMachineGraph() override = default;

    void draw()
    {
        ImGui::Begin(filename.value.c_str());

        ImGui::TextUnformatted("Node Graph");

        drawUIEmbedded();

        ImGui::End();
    }

    template<typename T>
    void load(const std::string& filepath, T t)
    {
        //We load the graph
        filename.setText(fs::path(filepath).filename().stem().string());
        int activeId = -1;
        StateMachineJsonExporter::DeserializeChainJson(
            filepath,
            getStateNodes(),
            getStateLinks(),
            activeId
        );

        auto view = views.findView<StateMachineView>();
        auto prelude = NodeGraphHelpers::build_lua_object_prelude(t, "t");
        view->setLuaConditionPrelude(prelude);
    }

private:
    void save()
    {
        if (EngineState::state == nullptr || filename.value.empty())
            return;

        const auto projectDir = std::filesystem::path(EngineState::state->currentActiveProjectDirectory);
        const auto assetsDir = projectDir / "Assets";
        const auto smFileName = filename.value + ".sm";
        const auto smPath = assetsDir / smFileName;

        Shared::WriteTextFile(smPath, json);

        const std::string guid = Shared::uuid();
        const auto metaPath = assetsDir / (guid + ".meta.json");
        const std::string metaJson =
            "{\n"
            "    \"guid\": \"" + guid + "\",\n"
            "    \"type\": \"statemachine\",\n"
            "    \"version\": \"0.1\",\n"
            "    \"content\": {\n"
            "        \"relative_path\": \"" + smFileName + "\"\n"
            "    }\n"
            "}\n";

        Shared::WriteTextFile(metaPath, metaJson);
    }

    UI::Shared::EditableText filename{};
    std::string json;
};

#endif // GLITTER_STATEMACHINEGRAPH_HPP

