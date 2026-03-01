//
// Created by subha on 01-03-2026.
//

#include "../Headers/UI/ModelUI/ModelUI.hpp"

#include "EngineState.hpp"
#include "imgui.h"
#include "3DModel/model.hpp"

UI::ModelUI::ModelUI()
{
    showUI = false;
    ModelName.setText("Model");
}

UI::ModelUI::~ModelUI()
= default;

void UI::ModelUI::start(const std::shared_ptr<Model>& model)
{
    selectedModel = model;
    materialListComponent.startMaterialsList();
    showUI = true;
}

void UI::ModelUI::draw()
{
    if (!showUI)
        return;

    if (ImGui::Begin(ModelName.value.c_str(), &showUI, ImGuiWindowFlags_AlwaysAutoResize))
    {
        materialListComponent.drawMaterialsList(selectedModel);

        if (ImGui::Button("Save"))
        {
            if (selectedModel != nullptr)
            {
                auto path = EngineState::navIntoProjectDir("Assets/");
                selectedModel->save(path);
            }
        }
    }
    ImGui::End();
}
