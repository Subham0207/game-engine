#include <UI/StatemachineUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>
#include <string>
#include "Helpers/Shared.hpp"
namespace SharedHelpers = Shared;
UI::StatemachineUI::StatemachineUI(){
      showUI = false;
}

void UI::StatemachineUI::draw()
{
    if(!showUI)
        return;

    if (ImGui::Begin("Create new Statemachine", &showUI))
    {
        ImGui::InputText("State Machine Name", stateMachineName, IM_ARRAYSIZE(stateMachineName));
        if (ImGui::Button("Create"))
        {
            const auto projectDir = std::filesystem::path(EngineState::state->currentActiveProjectDirectory);
            const auto assetsDir = projectDir / "Assets";
            const std::string guid = SharedHelpers::uuid();
            const auto metaPath = assetsDir / (guid + ".meta.json");
            const std::string metaJson =
                "{\n"
                "    \"guid\": \"" + guid + "\",\n"
                "    \"type\": \"statemachine\",\n"
                "    \"version\": \"0.1\",\n"
                "    \"content\": {\n"
                "        \"relative_path\": \"" + stateMachineName + "\"\n"
                "    }\n"
                "}\n";

            SharedHelpers::WriteTextFile(metaPath, metaJson);

            auto filename = std::string(stateMachineName) + ".sm";
            const auto smPath = assetsDir /  filename;
            SharedHelpers::WriteTextFile(smPath, "");

            if (EngineState::state != nullptr)
                EngineState::state->engineRegistry->update(guid, smPath.string());
        }
    }
    ImGui::End();
}

void UI::StatemachineUI::start()
{
    showUI = true;
}