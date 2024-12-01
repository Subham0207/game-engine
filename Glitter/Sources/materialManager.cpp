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

        getUIState().materialIndex = i;


        ImGui::Text("Albedo");
        ImGui::SameLine();
        if(materials[i]->albedo)
        ImGui::Text("%s", materials[i]->albedo->name);
        ImGui::SameLine();
        if(ImGui::Button(("MaterialBrowse##2" + std::to_string(i)).c_str()))
        {
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::albedoTexture;
            getUIState().selectAFile = true;
            getUIState().showOpenButton = true;
        }

        ImGui::Text("Normal");
        ImGui::SameLine();
        if(materials[i]->normal)
        ImGui::Text("%s", materials[i]->normal->name);
        ImGui::SameLine();
        if(ImGui::Button(("MaterialBrowse##3" + std::to_string(i)).c_str()))
        {
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::normalTexture;
            getUIState().selectAFile = true;
            getUIState().showOpenButton = true;
        }

        ImGui::Text("Metalness");
        ImGui::SameLine();
        if(materials[i]->metalness)
        ImGui::Text("%s", materials[i]->metalness->name);
        ImGui::SameLine();
        if(ImGui::Button(("MaterialBrowse##5"+ std::to_string(i)).c_str()))
        {
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::metalnessTexture;
            getUIState().selectAFile = true;
            getUIState().showOpenButton = true;
        }

        ImGui::Text("Roughness");
        ImGui::SameLine();
        if(materials[i]->roughness)
        ImGui::Text("%s", materials[i]->roughness->name);
        ImGui::SameLine();
        if(ImGui::Button(("MaterialBrowse##6"+ std::to_string(i)).c_str()))
        {
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::roughnessTexture;
            getUIState().selectAFile = true;
            getUIState().showOpenButton = true;
        }

        ImGui::Text("AO");
        ImGui::SameLine();
        if(materials[i]->ao)
        ImGui::Text("%s", materials[i]->ao->name);
        ImGui::SameLine();
        if(ImGui::Button(("MaterialBrowse##7"+ std::to_string(i)).c_str()))
        {
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::aoTexture;
            getUIState().selectAFile = true;
            getUIState().showOpenButton = true;
        }
    }
}
void UI::materialsFoundInModel()
{
    //We get the last model that was loaded and populate the UI
    Model* model = nullptr;
    if(getUIState().models.size() > 0)
        model = getUIState().models[getUIState().models.size() -  1];

    if(model != nullptr)
    {
        getUIState().materials = model->getMaterials();
    }
}