//
// Created by subha on 26-03-2026.
//

#include "../Headers/NodeGraph/FlowScript/StatemachineFlowScript.hpp"

StatemachineFlowScript::StatemachineFlowScript():
selectedLink(nullptr), showUI(false)
{

}

void StatemachineFlowScript::draw()
{
    if (!showUI || !selectedLink) return;

    ImGui::Begin("Statemachine flow script", &showUI);

    ImGui::TextUnformatted("Flow script");

    drawUIEmbedded();

    ImGui::End();
}

const std::string& StatemachineFlowScript::compile()
{
    FlowScript::compile();
    auto& luaCode = getCompiledLua();
    if (selectedLink)
    {
        selectedLink->condition = getCompiledLua();
    }
    return luaCode;
}
