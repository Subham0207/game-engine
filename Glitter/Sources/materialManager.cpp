#include <UI/materialManager.hpp>
#include <imgui.h>
#include "EngineState.hpp"
#include <Helpers/window3d.hpp>
#include <Modals/material.hpp>

void UI::renderMaterialManagerComponent()
{
    //TODO: (Do this first ) Show the texture in this channel as icon. Also has ability to change it.
    materialsFoundInModel();

    auto materials = getUIState().materials;
    for (size_t i = 0; i < materials.size(); i++)
    {
        ImGui::Text(("Material: " + std::to_string(i)).c_str());
        
        int textureIndex = 2; // This is just for UI purposes

        ImGui::Text("Albedo");
        UI::UpdateOrDisplayTexture(materials[i]->albedo,
        i,textureIndex,ProjectAsset::FileTypeOperation::albedoTexture);

        ImGui::Text("Normal");
        ImGui::SameLine();
        UI::UpdateOrDisplayTexture( materials[i]->normal,
        i,textureIndex+1,ProjectAsset::FileTypeOperation::normalTexture);

        ImGui::Text("Metalness");
        ImGui::SameLine();
        UI::UpdateOrDisplayTexture( materials[i]->metalness,
        i,textureIndex+2,ProjectAsset::FileTypeOperation::metalnessTexture);

        ImGui::Text("Roughness");
        ImGui::SameLine();
        UI::UpdateOrDisplayTexture( materials[i]->roughness,
        i,textureIndex+3,ProjectAsset::FileTypeOperation::roughnessTexture);

        ImGui::Text("AO");
        ImGui::SameLine();
        UI::UpdateOrDisplayTexture( materials[i]->ao,
        i,textureIndex+4,ProjectAsset::FileTypeOperation::aoTexture);
    }
}
void UI::materialsFoundInModel()
{
    //We get the last model that was loaded and populate the UI

    if(getUIState().selectedRenderableIndex > -1)
    {
        std::shared_ptr<Renderable> renderable = getActiveLevel().renderables[getUIState().selectedRenderableIndex];

        if(renderable != nullptr)
        {
            getUIState().materials = renderable->getMaterials();
        }
    }
}

void UI::UpdateOrDisplayTexture(
    std::shared_ptr<ProjectModals::Texture> texture,
    int materialIndex, int textureIndex,
    ProjectAsset::FileTypeOperation textureType
)
{
    ImGui::SameLine();
    if(texture)
    ImGui::Text("%s", texture->name.c_str());
    ImGui::SameLine();
    if(ImGui::Button(("MaterialBrowse##" + std::to_string(textureIndex) + std::to_string(materialIndex)).c_str()))
    {
        getUIState().fileTypeOperation = textureType;
        getUIState().selectAFile = true;
        getUIState().showOpenButton = true;
        getUIState().materialIndex = materialIndex;
    }
} 