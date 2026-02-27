//
// Created by subha on 26-02-2026.
//

#ifndef GLITTER_MATERIAL_HPP
#define GLITTER_MATERIAL_HPP
#pragma once
#include <map>
#include "IMaterial.hpp"
#include "Serializable.hpp"
#include "TextureUnits.hpp"

namespace Materials
{
    class Material: public IMaterial
    {
    public:
        Material()=default;
        Material(
            std::string filename,
            const std::string& vertexShaderFilePath,
            const std::string& fragmentShaderFilePath
            );

        Material(
            std::string filename,
            const std::string& vertexShaderFilePath,
            const std::string& fragmentShaderFilePath,
            const TextureUnits& textureUnits
            );

        ~Material() override;

        void Bind() override;

        [[nodiscard]] Shader* GetShader() const override;

        TextureUnits& GetTextureUnits() override;

        std::string GetClassId()const override{return "Material";};

        static std::shared_ptr<Material> loadMaterial(std::string guid);

    protected:
        const std::string typeName() const override {return "material"; }
        const std::string contentName() override {return mFilename; }

        void saveContent(fs::path contentFileLocation, std::ostream& os) override;
        void loadContent(fs::path contentFileLocation, std::istream& is) override;

    private:
        std::string mFilename;
        TextureUnits mTextureUnits;

        std::string mVertexShaderPath;
        std::string mFragmentShaderPath;
        std::unique_ptr<Shader> mShaderProgram;

        static std::map<std::string, std::shared_ptr<Material>> loadedMaterials;
    };
}


#endif //GLITTER_MATERIAL_HPP