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
namespace fs = std::filesystem;

std::map<std::string, std::shared_ptr<Materials::MaterialInstance>> Materials::MaterialInstance::loadedMaterialInstances;

namespace Materials
{
    MaterialInstance::MaterialInstance(std::string filename, const std::shared_ptr<Material>& material)
    {
        mFilename = filename;
        if (material) {
            mParentMaterial = material;
            textureUnits = mParentMaterial->GetTextureUnits();
            mParentMaterialAssetGuid = material->getAssetId();
        }
    }

    MaterialInstance::MaterialInstance(const std::string& filename, const std::string& parentMaterialGuid, const TextureUnits& units)
    {
        mFilename = filename;
        mParentMaterialAssetGuid = parentMaterialGuid;
        textureUnits = units;
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

    void MaterialInstance::update(const std::string& filename, const std::string& parentMaterialGuid,
        const TextureUnits& units)
    {
        mFilename = filename;
        mParentMaterialAssetGuid = parentMaterialGuid;
        textureUnits = units;
    }

    void MaterialInstance::saveContent(fs::path contentFileLocation, std::ostream& os)
    {
        if (mParentMaterialAssetGuid.empty())
            return;

        auto map = getEngineRegistryFilesMap();
        std::cout << "Master Material: " << mParentMaterialAssetGuid << std::endl;
        if (map.find(mParentMaterialAssetGuid) != map.end())
        {
            //Means parent material is not saved so save it
            auto path = contentFileLocation.parent_path();
            mParentMaterial->save(path);
        }

        bs::ptree root;
        // 1. Save Textures as a JSON Array
        bs::ptree textureNode;

        std::cout << "Processing MaterialInstance: " << contentFileLocation.string() << std::endl;
        auto putTexturesInJson = [&](const std::string& type, std::shared_ptr<ProjectModals::Texture> texture)
        {
            if (!texture || texture->name.empty())
                return;

            Shared::CopyFileToProjectDirectory(texture->name);

            bs::ptree texEntry;
            texEntry.put("type", type);
            texEntry.put("filepath", texture->name);

            // Push_back with an empty string key creates the array structure in JSON
            textureNode.push_back(std::make_pair("", texEntry));
        };

        //Handle when the textureUnits are not assigned. In that case we use Default Ids setup by the engine.
        putTexturesInJson("albedo", textureUnits.albedo);
        putTexturesInJson("normal", textureUnits.normal);
        putTexturesInJson("metalness", textureUnits.metalness);
        putTexturesInJson("roughness", textureUnits.roughness);
        putTexturesInJson("ao", textureUnits.ao);


        root.add_child("Textures", textureNode);
        root.put("parentMaterialAssetGuid", mParentMaterialAssetGuid);
        boost::property_tree::write_json(contentFileLocation.string(), root);
    }

    void MaterialInstance::loadContent(fs::path contentFileLocation, std::istream& is)
    {
        bs::ptree root;
        bs::read_json(contentFileLocation.string(), root);

        mFilename = contentFileLocation.filename().stem().string();

        if (auto texOpt = root.get_child_optional("Textures"))
        {
            for (auto& [key, info] : *texOpt)
            {
                std::string type = info.get<std::string>("type");
                auto path = fs::path(info.get<std::string>("filepath"));

                //TODO: Remove this patch...
                auto projectDir = fs::path(EngineState::state->currentActiveProjectDirectory);
                path = projectDir / "Assets" / path.filename();
                auto pathString = path.string();
                std::cout << "Loading texture: " << pathString << std::endl;

                // Load the actual pixel data
                int width, height, nrComponents;
                unsigned char* data = stbi_load(pathString.c_str(), &width, &height, &nrComponents, 0);

                if (data != nullptr) {
                    unsigned int id = Shared::sendTextureToGPU(data, width, height, nrComponents);
                    auto assign = [](const std::shared_ptr<ProjectModals::Texture>& texture, const unsigned int textureId, const std::string& path)
                    {
                        texture->id = textureId;
                        texture->name = path;
                    };

                    // Assign to the correct pointer based on the "type" string
                    if(type == "albedo")          assign(textureUnits.albedo, id, pathString);
                    else if (type == "normal")    assign(textureUnits.normal, id, pathString);
                    else if (type == "metalness") assign(textureUnits.metalness, id, pathString);
                    else if (type == "roughness") assign(textureUnits.roughness, id, pathString);
                    else if (type == "ao")        assign(textureUnits.ao, id, pathString);
                }
            }

            std::cout << "Loading Material" << mParentMaterialAssetGuid << std::endl;
            mParentMaterialAssetGuid = root.get<std::string>("parentMaterialAssetGuid");
            mParentMaterial =  Material::loadMaterial(mParentMaterialAssetGuid);
        }

    }
}
