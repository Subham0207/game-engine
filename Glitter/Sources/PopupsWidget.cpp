//
// Created by subha on 28-02-2026.
//

#include "../Headers/UI/AssetBrowser/PopupsWidget.hpp"

#include <memory>

#include "EngineState.hpp"
#include "imgui.h"
#include "Materials/Material.hpp"
#include "UI/materialManager.hpp"
#include "UI/AssetBrowser/constants.hpp"

namespace ProjectAsset
{
    void PopupsWidget::MaterialActionPopup(Asset selectedAsset)
    {
        if (ImGui::BeginPopup("MaterialPopup"))
        {
            if (ImGui::MenuItem("Edit in Material Editor"))
            {
                auto guid = fs::path(selectedAsset.filepath).filename().stem().stem().string();
                auto material = Materials::Material::loadMaterial(guid);
                getUIState().materialManagerUI->startMaterialEditor(material);
            }
            ImGui::EndPopup();
        }
    }

    void PopupsWidget::MaterialInstanceActionPopup(Asset selectedAsset)
    {
        if (ImGui::BeginPopup(UI::MATERIAL_INSTANCE_POPUP.c_str()))
        {
            if (ImGui::MenuItem("Edit in Material Instance Editor"))
            {
                auto guid = fs::path(selectedAsset.filepath).filename().stem().stem().string();
                auto materialInstance = Materials::MaterialInstance::loadMaterialInstance(guid);
                getUIState().materialInstanceEditorUI->start(materialInstance);
            }
            ImGui::EndPopup();
        }
    }
}
