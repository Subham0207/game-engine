#pragma once
#include <string>
#include <UIState/UIState.hpp>

namespace ProjectAsset{
    void RenderModelSelectionWindow(
        FileTypeOperation operation,
        bool showUI,
        std::string modelfileName,
        std::string albedo,
        std::string normal,
        std::string metalness,
        std::string roughness,
        std::string ao);

    void selectModelFile();
    void selectAlbedoTexture();
    void selectNormalTexture();
    void selectMetalTexture();
    void selectRoughnessTexture();
    void selectAOTexture();
};