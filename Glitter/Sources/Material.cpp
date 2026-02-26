//
// Created by subha on 26-02-2026.
//

#include "../Headers/Materials/Material.hpp"

namespace Materials
{
    Material::Material(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
    {
        textureUnits = TextureUnits();
        mShaderProgram = std::make_unique<Shader>(vertexShaderFilePath.c_str(), fragmentShaderFilePath.c_str());
    }

    Material::~Material()
    {
    }

    void Material::Bind() {
        mShaderProgram->use();
        TextureUnits::BindTextures(textureUnits);
    }

    Shader* Material::GetShader() const
    {
        return mShaderProgram.get(); // Returns the raw pointer safely
    }

    TextureUnits& Material::GetTextureUnits()
    {
        return textureUnits;
    }
}
