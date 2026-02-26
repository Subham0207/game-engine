//
// Created by subha on 26-02-2026.
//

#include "../Headers/Materials/TextureUnits.hpp"

#include "EngineState.hpp"

namespace Materials
{
    TextureUnits::TextureUnits()
    {
        albedo = std::make_shared<ProjectModals::Texture>();
        normal = std::make_shared<ProjectModals::Texture>();
        metalness = std::make_shared<ProjectModals::Texture>();
        roughness = std::make_shared<ProjectModals::Texture>();
        ao = std::make_shared<ProjectModals::Texture>();
    }

    TextureUnits::~TextureUnits()
    {
        albedo.reset();
        normal.reset();
        metalness.reset();
        roughness.reset();
        ao.reset();
    }

    void TextureUnits::BindTextures(const TextureUnits& textureUnits)
    {
        auto& ui = getUIState();

        // Helper to bind or fallback
        auto BindSlot = [&](unsigned int slot, std::shared_ptr<ProjectModals::Texture> tex, GLuint fallback) {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, (tex && tex->id != 0) ? tex->id : fallback);
        };

        BindSlot(1, textureUnits.albedo,    ui.whiteAOTextureID);
        BindSlot(2, textureUnits.normal,    ui.flatNormalTextureID);
        BindSlot(3, textureUnits.metalness, ui.nonMetalicTextureID); // Metalness usually defaults to 0 (black)
        BindSlot(4, textureUnits.roughness, ui.whiteAOTextureID); // Roughness usually defaults to 1 (white/rough)
        BindSlot(5, textureUnits.ao,        ui.whiteAOTextureID);
    }
}
