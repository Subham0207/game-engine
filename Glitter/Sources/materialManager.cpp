#include <UI/materialManager.hpp>
#include <imgui.h>
#include "EngineState.hpp"
#include <Helpers/window3d.hpp>

void UI::renderMaterialManagerComponent()
{
    //TODO: (Do this first ) Show the texture in this channel as icon. Also has ability to change it.
    textureFoundInModel();

    ImGui::Text("Albedo");
    ImGui::SameLine();
    ImGui::Text("%s", getUIState().albedo.c_str());
    ImGui::SameLine();
    if(ImGui::Button("MaterialBrowse##2"))
    {
        getUIState().selectAFile = true;
        getUIState().showOpenButton = true;
        getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::albedoTexture;
    }

    ImGui::Text("Normal");
    ImGui::SameLine();
    ImGui::Text("%s", getUIState().normal.c_str());
    ImGui::SameLine();
    if(ImGui::Button("MaterialBrowse##3"))
    {
        getUIState().selectAFile = true;
        getUIState().showOpenButton = true;
        getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::normalTexture;
    }

    ImGui::Text("Metalness");
    ImGui::SameLine();
    ImGui::Text("%s", getUIState().metalness.c_str());
    ImGui::SameLine();
    if(ImGui::Button("MaterialBrowse##5"))
    {
        getUIState().selectAFile = true;
        getUIState().showOpenButton = true;
        getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::metalnessTexture;
    }

    ImGui::Text("Roughness");
    ImGui::SameLine();
    ImGui::Text("%s", getUIState().roughness.c_str());
    ImGui::SameLine();
    if(ImGui::Button("MaterialBrowse##6"))
    {
        getUIState().selectAFile = true;
        getUIState().showOpenButton = true;
        getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::roughnessTexture;
    }

    ImGui::Text("AO");
    ImGui::SameLine();
    ImGui::Text("%s", getUIState().ao.c_str());
    ImGui::SameLine();
    if(ImGui::Button("MaterialBrowse##7"))
    {
        getUIState().selectAFile = true;
        getUIState().showOpenButton = true;
        getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::aoTexture;
    }
}
void UI::textureFoundInModel()
{
    //Texture loaded from ModelFile

    Model* model = nullptr;
    if(getUIState().models.size() > 0)
        model = getUIState().models[getUIState().models.size() -  1];

    if(model != nullptr)
    {
        auto textures = model->getTextures();
        for (size_t i = 0; i < textures.size(); i++)
        {
            switch (textures[i].type)
            {
                case aiTextureType_DIFFUSE:
                    getUIState().albedo = textures[i].name;
                    break;
                case aiTextureType_SPECULAR:
                    getUIState().roughness = textures[i].name;
                    break;
                case aiTextureType_NORMALS:
                    getUIState().normal = textures[i].name;
                    break;
                case aiTextureType_DIFFUSE_ROUGHNESS:
                    getUIState().roughness = textures[i].name;
                    break;
                case aiTextureType_AMBIENT_OCCLUSION:
                    getUIState().ao = textures[i].name;
                    break;
                case aiTextureType_METALNESS:
                    getUIState().metalness = textures[i].name;
                    break;
                
                default:
                    break;
            }
        }
    }
}