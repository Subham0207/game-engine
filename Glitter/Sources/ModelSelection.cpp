#pragma once
#include <UI/ModelPopUp/ModelSelection.hpp>
#include <EngineState.hpp>
#include <imgui.h>

namespace ProjectAsset{
    void RenderModelSelectionWindow(        
        FileTypeOperation operation,
        bool showUI,
        std::string modelfileName,
        std::string albedo,
        std::string normal,
        std::string metalness,
        std::string roughness,
        std::string ao)
    {
        if (ImGui::Begin("Asset Browser", &showUI))
        {
            //Start by populating the name of embedded textures if available on the model

            //If embedded texture was loaded or not; Allow user to overwrite/load new texture.
            ImGui::Text("Model");
            ImGui::SameLine();
            ImGui::Text("%s", modelfileName.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##1"))
            {
                selectModelFile();
            }

            ImGui::Text("Albedo");
            ImGui::SameLine();
            ImGui::Text("%s", albedo.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##2"))
            {
                selectAlbedoTexture();
            }

            ImGui::Text("Normal");
            ImGui::SameLine();
            ImGui::Text("%s", normal.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##3"))
            {
                selectNormalTexture();
            }

            ImGui::Text("Metalness");
            ImGui::SameLine();
            ImGui::Text("%s", metalness.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##5"))
            {
                selectMetalTexture();
            }

            ImGui::Text("Roughness");
            ImGui::SameLine();
            ImGui::Text("%s", roughness.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##6"))
            {
                selectRoughnessTexture();
            }

            ImGui::Text("AO");
            ImGui::SameLine();
            ImGui::Text("%s", ao.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Browse##7"))
            {
                selectAOTexture();
            }

            ImGui::End();
        }
    }
}

void ProjectAsset::selectModelFile()
{
    State::state->uiState.showFileDialog = true;
    State::state->uiState.showOpenButton = true;
    State::state->uiState.fileTypeOperation = FileTypeOperation::importModelFile;
}

void ProjectAsset::selectAlbedoTexture()
{
    State::state->uiState.showFileDialog = true;
    State::state->uiState.showOpenButton = true;
    State::state->uiState.fileTypeOperation = FileTypeOperation::albedoTexture;
}

void ProjectAsset::selectNormalTexture()
{
    State::state->uiState.showFileDialog = true;
    State::state->uiState.showOpenButton = true;
    State::state->uiState.fileTypeOperation = FileTypeOperation::normalTexture;
}

void ProjectAsset::selectMetalTexture()
{
    State::state->uiState.showFileDialog = true;
    State::state->uiState.showOpenButton = true;
    State::state->uiState.fileTypeOperation = FileTypeOperation::metalnessTexture;
}

void ProjectAsset::selectRoughnessTexture()
{
    State::state->uiState.showFileDialog = true;
    State::state->uiState.showOpenButton = true;
    State::state->uiState.fileTypeOperation = FileTypeOperation::roughnessTexture;
}

void ProjectAsset::selectAOTexture()
{
    State::state->uiState.showFileDialog = true;
    State::state->uiState.showOpenButton = true;
    State::state->uiState.fileTypeOperation = FileTypeOperation::aoTexture;
}
