//
// Created by subha on 01-03-2026.
//

#include "../Headers/UI/MaterialListComponent.hpp"
#include <filesystem>

#include "EngineState.hpp"
#include "UI/Shared/ComboUI.hpp"
#include "UI/Shared/Utils.hpp"
namespace fs = std::filesystem;

UI::MaterialListComponent::MaterialListComponent()
= default;

void UI::MaterialListComponent::startMaterialsList()
{
    if (materialsListInitialized)
        return;

    materialsList.materialGuids.clear();
    materialsList.materialNames.clear();

    materialsListInitialized = true;
    std::vector<fs::path> searchPaths = {EngineState::state->currentActiveProjectDirectory};
    auto materialfilesMap = EngineState::state->engineRegistry->materialFileMap;
    auto materialInstancefilesMap = EngineState::state->engineRegistry->materialInstanceFileMap;
    for (auto &[key, value] : materialfilesMap)
    {
        materialsList.materialGuids.push_back(key);
        materialsList.materialNames.push_back(value);
    }

    for (auto &[key, value] : materialInstancefilesMap)
    {
        materialsList.materialGuids.push_back(key);
        materialsList.materialNames.push_back(value);
    }
}

void UI::MaterialListComponent::drawMaterialsList(Model* selectedModel)
{
    if (selectedModel != nullptr)
    {
        //Need to track materialguids List and materialName list. Material list is what we show but use guid to assign it mesh.
        //Use combo UI to list materials found in the project and which one is in use for the model's mesh should appear as active else none.
        for (int i = 0;i < selectedModel->meshes.size(); i++)
        {
            auto& mesh = selectedModel->meshes[i];
            auto indexOfMaterialInList = 0;
            auto it = std::find(materialsList.materialGuids.begin(), materialsList.materialGuids.end(),mesh.materialAssetGuid);
            if (it != materialsList.materialGuids.end())
            {
                indexOfMaterialInList = std::distance(materialsList.materialGuids.begin(), it);
                indexOfMaterialInList = Utils::toUiIndex(indexOfMaterialInList);
            }

            ImGui::PushItemWidth(200.0f);
            UI::Shared::comboUI(
                ("Material"s + std::to_string(i)).c_str(),
                indexOfMaterialInList,
                materialsList.materialNames
                );
            if(ImGui::Button("Refresh"))
            {
                materialsListInitialized = false;
            }
            ImGui::PopItemWidth();

            //Load the selected material on the mesh...
            auto actualIndex = Utils::toDataTypeIndex(indexOfMaterialInList);
            if (actualIndex != -1)
            {
                auto assetGuid = materialsList.materialGuids[actualIndex];
                if (assetGuid != mesh.materialAssetGuid)
                {
                    mesh.materialAssetGuid = assetGuid;
                    mesh.setupMaterial();
                }
            }
        }
    }

}
