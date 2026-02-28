//
// Created by subha on 28-02-2026.
//

#include "../Headers/UI/MaterialInstanceEditor.hpp"

#include "EngineState.hpp"
#include "UI/FileExplorer.hpp"
#include "UI/Shared/ComboUI.hpp"
#include "UI/Shared/Utils.hpp"

UI::MaterialInstanceEditor::MaterialInstanceEditor() : showUI(false), materialInstanceUIModal{},
                                                       openFileExplorer(false)
{
    materialInstanceName.setText("Material Instance");
    materialsListInitialized = false;
    operatingOnPath = nullptr;
}

void UI::MaterialInstanceEditor::setShowUI(bool show)
{
    showUI = show;
}

void UI::MaterialInstanceEditor::start(std::shared_ptr<Materials::MaterialInstance> materialInstance)
{
    if (materialsListInitialized)
        return;

    materialsListInitialized = true;
    std::vector<fs::path> searchPaths = {EngineState::state->currentActiveProjectDirectory};
    auto filesMap = EngineState::state->engineRegistry->materialFileMap;
    for (auto &[key, value] : filesMap)
    {
        materialsList.materialGuids.push_back(key);
        materialsList.materialNames.push_back(value);
    }

    if (materialInstance)
    {
        auto findIndex = [](std::vector<std::string>& list, std::string item)
        {
            if (const auto it = std::find(list.begin(),list.end(),item); it != list.end())
            {
                return std::distance(list.begin(), it);
            }
            return -1LL;
        };

        materialInstanceRef = materialInstance;

        materialInstanceName.setText(materialInstance->contentName());
        materialInstanceUIModal.albedoMapLocation = materialInstance->GetTextureUnits().albedo->name;
        materialInstanceUIModal.normalMapLocation = materialInstance->GetTextureUnits().normal->name;
        materialInstanceUIModal.metallicMapLocation = materialInstance->GetTextureUnits().metalness->name;
        materialInstanceUIModal.roughnessMapLocation = materialInstance->GetTextureUnits().roughness->name;
        materialInstanceUIModal.aoMapLocation = materialInstance->GetTextureUnits().ao->name;

        materialInstanceUIModal.parentMaterialIndex =  Utils::toUiIndex(findIndex(materialsList.materialGuids, materialInstance->getParentMaterialGuid()));
    }

    showUI = true;
}

void UI::MaterialInstanceEditor::drawUI()
{
    if (!showUI)
        return;
    if (ImGui::Begin(materialInstanceName.value.c_str(), &showUI))
    {
        auto DrawField = [this](const char* label, std::string& path) {
            ImGui::Text("%s:", label);

            // Show the path or a placeholder if empty
            if (path.empty()) {
                ImGui::TextDisabled("  (No file selected)");
            } else {
                // Using Bullet to indent the path slightly for visual hierarchy
                ImGui::BulletText("%s", path.c_str());
            }

            ImGui::SameLine(ImGui::GetWindowWidth() - 70); // Align button to the right
            std::string btnLabel = "Browse##" + std::string(label);

            if (ImGui::Button(btnLabel.c_str())) {
                operatingOnPath = &path;
                openFileExplorer = true;
            }
            ImGui::Spacing();
        };

        Shared::EditableTextUI("MaterialName", materialInstanceName);
        ImGui::SeparatorText("Maps");
        DrawField("Albedo", materialInstanceUIModal.albedoMapLocation);
        DrawField("Normal", materialInstanceUIModal.normalMapLocation);
        DrawField("Metallic", materialInstanceUIModal.metallicMapLocation);
        DrawField("Roughness", materialInstanceUIModal.roughnessMapLocation);
        DrawField("AO", materialInstanceUIModal.aoMapLocation);

        ImGui::SeparatorText("Shaders");
        UI::Shared::comboUI(
            "Materials",
            materialInstanceUIModal.parentMaterialIndex,
            materialsList.materialNames
        );

        ImGui::Separator();
        if (ImGui::Button("Save Material", ImVec2(-FLT_MIN, 0))) {
            // Save logic
            //create a Material object and save that...
            const Materials::TextureUnits units;
            units.albedo->name = materialInstanceUIModal.albedoMapLocation;
            units.normal->name = materialInstanceUIModal.normalMapLocation;
            units.metalness->name = materialInstanceUIModal.metallicMapLocation;
            units.roughness->name = materialInstanceUIModal.roughnessMapLocation;
            units.ao->name = materialInstanceUIModal.aoMapLocation;
            auto materialInstance = materialInstanceRef ? materialInstanceRef : std::make_unique<Materials::MaterialInstance>(
                materialInstanceName.value,
                materialsList.materialGuids[Utils::toDataTypeIndex(materialInstanceUIModal.parentMaterialIndex)],
                units
                );
            auto dir = EngineState::state->navIntoProjectDir("Assets");
            materialInstance->save(dir);

            //cleanup
            materialsList.clear();
        }
    }
    ImGui::End();

    if (!openFileExplorer)
        return;
    if(ImGui::Begin("FileExplorer", &openFileExplorer))
    {
        ProjectAsset::RenderFileExplorer(getUIState().currentPath, EngineState::state->uiState.fileNames);

        if (ImGui::Button("Open"))
        {
            *operatingOnPath = getUIState().filePath;
            openFileExplorer = false;
            operatingOnPath = nullptr;
        }

    ImGui::End();
    }


}
