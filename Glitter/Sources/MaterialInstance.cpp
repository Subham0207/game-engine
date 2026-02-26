//
// Created by subha on 26-02-2026.
//

#include "../Headers/Materials/MaterialInstance.hpp"

namespace Materials
{
    MaterialInstance::MaterialInstance(const std::shared_ptr<Material>& material)
    {
        mParentMaterial = material;
        if (mParentMaterial) {
            textureUnits = mParentMaterial->GetTextureUnits();
        }
    }

    void MaterialInstance::Bind() {
        mParentMaterial->GetShader()->use();
        TextureUnits::BindTextures(textureUnits);
    }

    Shader* MaterialInstance::GetShader() const
    {
        return mParentMaterial ? mParentMaterial->GetShader() : nullptr;
    }

    TextureUnits& MaterialInstance::GetTextureUnits()
    {
        return textureUnits;
    }
}
