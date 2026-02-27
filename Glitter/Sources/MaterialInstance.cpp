//
// Created by subha on 26-02-2026.
//

#include "../Headers/Materials/MaterialInstance.hpp"

#include "EngineState.hpp"
#include "stb_image.h"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"
#include "Helpers/Shared.hpp"
namespace bs = boost::property_tree;

std::map<std::string, std::shared_ptr<Materials::MaterialInstance>> Materials::MaterialInstance::loadedMaterialInstances;

namespace Materials
{
    MaterialInstance::MaterialInstance(std::string filename, const std::shared_ptr<Material>& material)
    {
        mFilename = filename;
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

    std::shared_ptr<MaterialInstance> MaterialInstance::loadMaterialInstance(std::string guid)
    {
        if (loadedMaterialInstances.find(guid) != loadedMaterialInstances.end())
            return loadedMaterialInstances[guid];

        auto parentPath = fs::path(getEngineRegistryFilesMap()[guid]).parent_path();
        auto materialInstance = std::make_shared<MaterialInstance>();
        materialInstance->load(parentPath, guid);
        loadedMaterialInstances[guid] = materialInstance;

        return materialInstance;
    }

    void MaterialInstance::saveContent(fs::path contentFileLocation, std::ostream& os)
    {
        if (mParentMaterial == nullptr || mParentMaterialAssetGuid.empty())
            return;

        bs::ptree root;
        // 1. Save Textures as a JSON Array
        bs::ptree textureNode;

        auto putTexturesInJson = [&](const std::string& type, const std::string& filepath)
        {
            bs::ptree texEntry;
            texEntry.put("type", type);
            texEntry.put("filepath", filepath);

            // Push_back with an empty string key creates the array structure in JSON
            textureNode.push_back(std::make_pair("", texEntry));
        };

        //Handle when the the textureUnits are not assigned. In that case we use Default Ids setup by the engine.
        putTexturesInJson("albedo", textureUnits.albedo->name);
        putTexturesInJson("normal", textureUnits.normal->name);
        putTexturesInJson("metalness", textureUnits.metalness->name);
        putTexturesInJson("roughness", textureUnits.roughness->name);
        putTexturesInJson("ao", textureUnits.ao->name);


        root.add_child("Textures", textureNode);
        root.put("parentMaterialAssetGuid", mParentMaterialAssetGuid);
        boost::property_tree::write_json(contentFileLocation.string(), root);
    }

    void MaterialInstance::loadContent(fs::path contentFileLocation, std::istream& is)
    {
        bs::ptree root;
        bs::read_json(contentFileLocation.string(), root);

        auto getTextureId = [&](const std::string& jsonKey)
        {
            auto filePath = root.get<std::string>(jsonKey);
            int width, height, nrComponents;
            unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrComponents, 0);
            return Shared::sendTextureToGPU(data, width, height, nrComponents);
        };

        textureUnits.albedo->id = getTextureId("Textures.albedo");
        textureUnits.normal->id = getTextureId("Textures.normal");
        textureUnits.metalness->id = getTextureId("Textures.metalness");
        textureUnits.roughness->id = getTextureId("Textures.roughness");
        textureUnits.ao->id = getTextureId("Textures.ao");



    }
}
