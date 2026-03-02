//
// Created by subha on 26-02-2026.
//

#include <utility>

#include "../Headers/Materials/Material.hpp"

#include "EngineState.hpp"
#include "boost/property_tree/ptree.hpp"
#include "Helpers/Shared.hpp"
namespace bs = boost::property_tree;

std::map<std::string, std::shared_ptr<Materials::Material>> Materials::Material::loadedMaterials;

namespace Materials
{
    Material::Material(std::string filename, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
    {
        mFilename = std::move(filename);
        mTextureUnits = TextureUnits();
        mVertexShaderPath = vertexShaderFilePath;
        mFragmentShaderPath = fragmentShaderFilePath;
        mShaderProgram = std::make_unique<Shader>(vertexShaderFilePath.c_str(), fragmentShaderFilePath.c_str());
    }

    Material::Material(std::string filename, const std::string& vertexShaderFilePath,
        const std::string& fragmentShaderFilePath, const TextureUnits& textureUnits)
    {
        mFilename = std::move(filename);
        mTextureUnits = textureUnits;
        mVertexShaderPath = vertexShaderFilePath;
        mFragmentShaderPath = fragmentShaderFilePath;
        mShaderProgram = std::make_unique<Shader>(vertexShaderFilePath.c_str(), fragmentShaderFilePath.c_str());

    }

    Material::~Material()
    {
    }

    void Material::Bind() {
        mShaderProgram->use();
        TextureUnits::BindTextures(mTextureUnits);
    }

    Shader* Material::GetShader() const
    {
        return mShaderProgram.get(); // Returns the raw pointer safely
    }

    TextureUnits& Material::GetTextureUnits()
    {
        return mTextureUnits;
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

    void Material::Update(std::string filename, const std::string& vertexShaderFilePath,
        const std::string& fragmentShaderFilePath, const TextureUnits& textureUnits)
    {
        mFilename = std::move(filename);
        mTextureUnits = textureUnits;
        mVertexShaderPath = vertexShaderFilePath;
        mFragmentShaderPath = fragmentShaderFilePath;
        mShaderProgram = std::make_unique<Shader>(vertexShaderFilePath.c_str(), fragmentShaderFilePath.c_str());
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

        //Handle texture units being empty, In which case we use default setup by the engine.
        putTexturesInJson("albedo", mTextureUnits.albedo->name);
        putTexturesInJson("normal", mTextureUnits.normal->name);
        putTexturesInJson("metalness", mTextureUnits.metalness->name);
        putTexturesInJson("roughness", mTextureUnits.roughness->name);
        putTexturesInJson("ao", mTextureUnits.ao->name);


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

        mFilename = contentFileLocation.filename().stem().string();

        // 1. Loop through the "Textures" array
        if (auto texOpt = root.get_child_optional("Textures"))
        {
            for (auto& [key, info] : *texOpt)
            {
                std::string type = info.get<std::string>("type");
                std::string path = info.get<std::string>("filepath");

                // Load the actual pixel data
                int width, height, nrComponents;
                unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);

                if (data != nullptr) {
                    unsigned int id = Shared::sendTextureToGPU(data, width, height, nrComponents);
                    auto assign = [](const std::shared_ptr<ProjectModals::Texture>& texture, const unsigned int textureId, const std::string& path)
                    {
                        texture->id = textureId;
                        texture->name = path;
                    };

                    // Assign to the correct pointer based on the "type" string
                    if(type == "albedo")          assign(mTextureUnits.albedo, id, path);
                    else if (type == "normal")    assign(mTextureUnits.normal, id, path);
                    else if (type == "metalness") assign(mTextureUnits.metalness, id, path);
                    else if (type == "roughness") assign(mTextureUnits.roughness, id, path);
                    else if (type == "ao")        assign(mTextureUnits.ao, id, path);
                }
            }
        }

        // 2. Load Shaders
        mVertexShaderPath = root.get<std::string>("Shader.vertexShaderPath");
        mFragmentShaderPath = root.get<std::string>("Shader.fragmentShaderPath");
        mShaderProgram = std::make_unique<Shader>(mVertexShaderPath.c_str(), mFragmentShaderPath.c_str());
    }
}
