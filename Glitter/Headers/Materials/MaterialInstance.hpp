//
// Created by subha on 26-02-2026.
//

#ifndef GLITTER_MATERIALINSTANCE_HPP
#define GLITTER_MATERIALINSTANCE_HPP
#include <map>

#include "IMaterial.hpp"
#include "TextureUnits.hpp"
#include "Material.hpp"

namespace Materials
{
    class MaterialInstance: public IMaterial
    {
    public:
        MaterialInstance(const std::shared_ptr<Material>& material);
        ~MaterialInstance() override= default;
        void Bind() override;
        [[nodiscard]] Shader* GetShader() const override;
        TextureUnits& GetTextureUnits() override;

    private:
        TextureUnits textureUnits;
        std::shared_ptr<Material> mParentMaterial;

        static std::map<std::string, std::shared_ptr<MaterialInstance>> assetIdToRefMap;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & textureUnits;
        }

    };
}



#endif //GLITTER_MATERIALINSTANCE_HPP