//
// Created by subha on 26-02-2026.
//

#ifndef GLITTER_MATERIAL_HPP
#define GLITTER_MATERIAL_HPP
#include <map>

#include "IMaterial.hpp"
#include "TextureUnits.hpp"

namespace Materials
{
    class Material: public IMaterial
    {
    public:
        Material(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);

        ~Material() override;

        void Bind() override;

        [[nodiscard]] Shader* GetShader() const override;

        TextureUnits& GetTextureUnits() override;

    private:
        TextureUnits textureUnits;
        std::unique_ptr<Shader> mShaderProgram;

        static std::map<std::string, std::shared_ptr<Material>> assetIdToRefMap;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & textureUnits;
        }
    };
}


#endif //GLITTER_MATERIAL_HPP