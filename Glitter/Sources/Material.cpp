//
// Created by subha on 26-02-2026.
//

#include <utility>

#include "../Headers/Materials/Material.hpp"

#include "EngineState.hpp"
#include "boost/property_tree/ptree.hpp"
#include "Helpers/Shared.hpp"
namespace bs = boost::property_tree;

namespace Materials
{
    Material::Material(std::string filename, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
    {
        mFilename = std::move(filename);
        textureUnits = TextureUnits();
        mVertexShaderPath = vertexShaderFilePath;
        mFragmentShaderPath = fragmentShaderFilePath;
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

    std::shared_ptr<Material> Material::loadMaterial(std::string guid)
    {
        if (loadedMaterials.find(guid) != loadedMaterials.end())
            return loadedMaterials[guid];

        auto parentPath = fs::path(getEngineRegistryFilesMap()[guid]).parent_path();
        auto material = std::make_shared<Material>();
        material->load(parentPath, guid);
        loadedMaterials[guid] = material;

        return material;
    }

    void Material::saveContent(fs::path contentFileLocation, std::ostream& os)
    {
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

        putTexturesInJson("albedo", textureUnits.albedo->name);
        putTexturesInJson("normal", textureUnits.normal->name);
        putTexturesInJson("metalness", textureUnits.metalness->name);
        putTexturesInJson("roughness", textureUnits.roughness->name);
        putTexturesInJson("ao", textureUnits.ao->name);


        root.add_child("Textures", textureNode);

        // 2. Save Shader as an Object
        bs::ptree shaderNode;
        shaderNode.put("vertexShaderPath", mVertexShaderPath);
        shaderNode.put("fragmentShaderPath", mFragmentShaderPath);
        root.add_child("Shader", shaderNode);

        // 3. Write to the output stream
        write_json(contentFileLocation.string(), root);
    }

    void Material::loadContent(fs::path contentFileLocation, std::istream& is)
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

        mVertexShaderPath = root.get<std::string>("Shader.vertexShaderPath");
        mFragmentShaderPath = root.get<std::string>("Shader.fragmentShaderPath");

        mShaderProgram = std::make_unique<Shader>(mVertexShaderPath.c_str(), mFragmentShaderPath.c_str());
    }
}
