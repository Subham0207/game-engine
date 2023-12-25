#pragma once
#include <vector>
#include <string>
#include <shader.hpp>
#include <mesh.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"


unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

class Model
{
public:
    Model(char* path)
    {
        loadModel(path);
    }
    void Draw(Shader* shader);
    aiAABB* GetBoundingBox();
    void LoadTexture(std::string texturePath, std::string typeName);
private:
    // model data
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;
    aiAABB* boundingBox;

    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
        std::string typeName);
    void Model::calculateBoundingBox(const aiScene* scene);
    
};