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
#include "UI/ModelUI/ModelUI.hpp"

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

    void PopupsWidget::ModelActionPopup(Asset selectedAsset)
    {
        if (ImGui::BeginPopup(UI::MODEL_POPUP.c_str()))
        {
            if (ImGui::MenuItem("Edit in Model file Editor"))
            {
                auto guid = fs::path(selectedAsset.filepath).filename().stem().stem().string();
                auto filesMap = getEngineRegistryFilesMap();
                if (auto it = filesMap.find(guid); it != filesMap.end())
                {
                    auto modelPath = fs::path(selectedAsset.filepath).parent_path();
                    auto model = Model::loadWithClassFactory(modelPath, guid);
                    model->setModelMatrix(glm::identity<glm::mat4>());
                    auto filename = fs::path(filesMap[guid]).filename().stem().string();
                    model->setFileName(filename);
                    getUIState().modelUIState->start(model);
                }
            }

            if (ImGui::MenuItem("Add Model to current level"))
            {
                auto guid = fs::path(selectedAsset.filepath).filename().stem().stem().string();
                auto filesMap = getEngineRegistryFilesMap();
                if (auto it = filesMap.find(guid); it != filesMap.end())
                {
                    auto modelPath = fs::path(selectedAsset.filepath).parent_path();
                    auto model = Model::loadWithClassFactory(modelPath, guid);
                    model->setModelMatrix(glm::identity<glm::mat4>());
                    auto filename = fs::path(filesMap[guid]).filename().stem().string();
                    model->setFileName(filename);
                    getActiveLevel().addRenderable(model);
                }
            }
            ImGui::EndPopup();
        }
    }
}
