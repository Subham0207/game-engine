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
    class MaterialInstance: public IMaterial, public Serializable
    {
    public:
        std::string GetClassId() const override{return "MaterialInstance";}

        MaterialInstance()=default;
        MaterialInstance(const std::shared_ptr<Material>& material);
        ~MaterialInstance() override= default;
        void Bind() override;
        [[nodiscard]] Shader* GetShader() const override;
        TextureUnits& GetTextureUnits() override;

        static std::shared_ptr<MaterialInstance> loadMaterialInstance(std::string guid);
    protected:
        const std::string typeName() const override {return "materialInstance"; }
        const std::string contentName() override {return mFilename; }

        void saveContent(fs::path contentFileLocation, std::ostream& os) override;
        void loadContent(fs::path contentFileLocation, std::istream& is) override;
    private:
        std::string mFilename;
        TextureUnits textureUnits;

        std::string mParentMaterialAssetGuid;
        std::shared_ptr<Material> mParentMaterial;

        static std::map<std::string, std::shared_ptr<MaterialInstance>> loadedMaterialInstances;
    };
}



#endif //GLITTER_MATERIALINSTANCE_HPP