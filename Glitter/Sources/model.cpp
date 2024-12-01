#pragma once
#include "3DModel/model.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include "Helpers/AssimpGLMHelpers.hpp"
#include <Helpers/vertexBoneDataHelper.hpp>
#include "Helpers/Shared.hpp"
#include <Modals/vertex.hpp>
#include <stb_image.h>
#include <stb_image_write.h>
// using namespace std;
namespace fs = std::filesystem;

void Model::loadModel(
    std::string path,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    boundingBox = new aiAABB();
    calculateBoundingBox(scene);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    //Load textures
    // for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
    //     aiMaterial* material = scene->mMaterials[i];
    //     loadMaterialTextures(material, aiTextureType_DIFFUSE);
    //     loadMaterialTextures(material, aiTextureType_SPECULAR);
    // }

    //Embedded Texture
    //Instead of processing texture we can process material to also know what type of the texture that is ??
    for (unsigned int m = 0; m < scene->mNumMaterials; m++) {
        aiMaterial* material = scene->mMaterials[m];

        // Check for different texture types
        const std::vector<aiTextureType> textureTypes = {
            aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_NORMALS, aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_AMBIENT_OCCLUSION,
            aiTextureType_METALNESS
        };

        for (aiTextureType type : textureTypes) {
            aiString texturePath;
            if (material->GetTexture(type, 0, &texturePath) == AI_SUCCESS) {
                std::cout << "Material " << m << " has texture type " << type << ": " << texturePath.C_Str() << std::endl;

                // Look for the corresponding embedded texture
                for (unsigned int t = 0; t < scene->mNumTextures; t++) {
                    const aiTexture* embeddedTexture = scene->mTextures[t];
                    if (texturePath.C_Str() == std::string(embeddedTexture->mFilename.C_Str())) {
                        std::cout << "Found embedded texture matching " << texturePath.C_Str() << std::endl;
                        // Process embedded texture
                        if (embeddedTexture->mHeight == 0) {
                        std::cout << "Compressed texture format: " << embeddedTexture->achFormatHint << std::endl;
                        std::cout << "Size: " << embeddedTexture->mWidth << " bytes" << std::endl;
                        if(embeddedTexture->pcData)
                        if(type == aiTextureType_SPECULAR)
                            loadEmbeddedTexture(embeddedTexture, aiTextureType_DIFFUSE_ROUGHNESS);
                        loadEmbeddedTexture(embeddedTexture, type);
                        } else {
                            std::cout << "Uncompressed texture" << std::endl;
                            std::cout << "Dimensions: " << embeddedTexture->mWidth << " x " << embeddedTexture->mHeight << std::endl;
                        }
                    }
                }
            }
        }
    }
    // processTexture(scene);
    processNode(scene->mRootNode, scene, m_BoneInfoMap, m_BoneCounter);
}

void Model::saveSerializedModel(std::string filename, Model &model)
{
    fs::path dir = fs::path(filename).parent_path();
    if (dir.empty()) {
        // Set the directory to the current working directory
        dir = fs::current_path();
    }
    if (!fs::exists(dir)) {
        if (!fs::create_directories(dir)) {
            std::cerr << "Failed to create directories: " << dir << std::endl;
            return;
        }
    }
    model.directory = filename;
    std::ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);
    oa << model;
    ofs.close();
    //Texture-ids needs to  generated again
    //They will need to bound again to GPU
}

void Model::processTexture(const aiScene *scene)
{
    if (scene->HasTextures()) {
        for (unsigned int i = 0; i < scene->mNumTextures; i++) {
            const aiTexture* texture = scene->mTextures[i];
            std::cout << "Embedded Texture " << i << ": " << std::endl;

            // Check if the texture is compressed or not
            if (texture->mHeight == 0) {
                std::cout << "Compressed texture format: " << texture->achFormatHint << std::endl;
                std::cout << "Size: " << texture->mWidth << " bytes" << std::endl;
                if(texture->pcData)
                loadEmbeddedTexture(texture, aiTextureType_DIFFUSE);
            } else {
                std::cout << "Uncompressed texture" << std::endl;
                std::cout << "Dimensions: " << texture->mWidth << " x " << texture->mHeight << std::endl;
            }

            // Optionally, write the texture to a file to inspect it
            // std::ofstream outFile("embedded_texture_" + std::to_string(i) + ".data", std::ios::binary);
            // outFile.write(reinterpret_cast<const char*>(texture->pcData), texture->mWidth * texture->mHeight * 4);
            // outFile.close();
        }
    } else {
        std::cout << "No embedded textures found." << std::endl;
    }
}

void Model::loadEmbeddedTexture(const aiTexture* texture, aiTextureType textureType)
{
    int mWidth, mheight, nrComponents;
    unsigned char* data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(texture->pcData), texture->mWidth, &mWidth, &mheight, &nrComponents, 0);
    auto filename = "Assets/" + fs::path(texture->mFilename.C_Str()).filename().string();
    stbi_write_png(filename.c_str(), mWidth, mheight, nrComponents, data, 0);
    unsigned int textureID = Shared::sendTextureToGPU(data, mWidth, mheight, nrComponents);
    auto filepath = fs::current_path().append(filename).string();
    ProjectModals::Texture newTexture(textureID, textureType, filepath);
    textureIds.push_back(newTexture);
}

void Model::processNode(
    aiNode* node,
     const aiScene* scene,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        // Store the information about a mesh
        // that we can comprehend right now and push into an array
        meshes.push_back(processMesh(mesh, scene, m_BoneInfoMap, m_BoneCounter));
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, m_BoneInfoMap, m_BoneCounter);
    }
}

Mesh Model::processMesh(
    aiMesh* mesh,
    const aiScene* scene,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter)
{
   std::vector<ProjectModals::Vertex> vertices;
   std::vector<unsigned int>indices;
   std::vector<ProjectModals::Texture> textures;

    //1. Procesing vertices
   for(unsigned int i = 0; i < mesh->mNumVertices; i++)
   {
    ProjectModals::Vertex vertex;
    vertex.Position = glm::vec3(
     static_cast<float>(mesh->mVertices[i].x),
     static_cast<float>(mesh->mVertices[i].y),
     static_cast<float>(mesh->mVertices[i].z)
    );

    if(mesh->HasNormals())
    {
        vertex.Normal = glm::vec3(
        static_cast<float>(mesh->mNormals[i].x),
        static_cast<float>(mesh->mNormals[i].y),
        static_cast<float>(mesh->mNormals[i].z)
        );
    }

    //A mesh can contain up to 8 different texture coordintes for now we are considering only the first one
    if(mesh->HasTextureCoords(0))
    {
        vertex.TexCoords = glm::vec2(
        static_cast<float>(mesh->mTextureCoords[0][i].x),
        static_cast<float>(mesh->mTextureCoords[0][i].y)
        );
    }
    else{
    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    }

    if(mesh->HasTangentsAndBitangents())
    {
        vertex.Tangent = glm::vec3(
            mesh->mTangents[i].x,
            mesh->mTangents[i].y,
            mesh->mTangents[i].z
        );

        vertex.Bitangent = glm::vec3(
            mesh->mBitangents[i].x,
            mesh->mBitangents[i].y,
            mesh->mBitangents[i].z
        );
    }

    //How many index can exist; Assuming only first index is filled
    if(mesh->HasVertexColors(0))
    {
     //Pass the color value to shader -- but how ? maybe its stored in vertex
     vertex.Color = glm::vec4(
        mesh->mColors[0][i].r,
        mesh->mColors[0][i].g,
        mesh->mColors[0][i].b,
        mesh->mColors[0][i].a
     );
    }
    else{
        // std::cout << "Vertex Color set to red" << std::endl;
        vertex.Color = glm::vec4(1.0f,0.0f,0.0f,1.0f);
    }

    vertices.push_back(vertex);
   }

    //2. Processing Indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    } 

    //3. process materials
    // What if there are no textures found there is no reason to error out on that case
    // Sample color instead so that will be vertex color
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    Helpers::ExtractBoneWeightForVertices(vertices,mesh,scene, *m_BoneInfoMap, *m_BoneCounter);
    return Mesh(vertices, indices, &textureIds);
}

std::string GetAbsoluteTexturePath(const std::string& modelDirectory, const aiString& relativePath) {
    // std::string modelDir = std::filesystem::path(modelDirectory).parent_path().string();
    std::stringstream ss;
    ss << modelDirectory << "/" << relativePath.C_Str();
    return ss.str();
}
void Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type)
{
    // The relative texture path we get is from the model's location.
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        if (mat->GetTexture(type, i, &str) == AI_SUCCESS) {
            std::string absoluteTexturePath = "Assets/"+GetAbsoluteTexturePath(directory, str);
            unsigned int textureId = Shared::TextureFromFile(absoluteTexturePath.c_str(), absoluteTexturePath);
            // textureIds.push_back(textureId);
        }
    }
    
}

void Model::Draw(Shader* shader, GLFWwindow* window)
{
    //Probably this should happen if the model is selected but its fine as a start
    if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        this->whichTransformActive = ImGuizmo::OPERATION::TRANSLATE;
    }
    if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        this->whichTransformActive = ImGuizmo::OPERATION::ROTATE;
    }
    if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        this->whichTransformActive = ImGuizmo::OPERATION::SCALE;
    }

	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader);
}

aiAABB* Model::GetBoundingBox(){
    return boundingBox;
}

void Model::calculateBoundingBox(const aiScene* scene) {
    // Initialize min and max with extreme values
    boundingBox->mMin.x = boundingBox->mMin.y = boundingBox->mMin.z =  1e10f;
    boundingBox->mMax.x = boundingBox->mMax.y = boundingBox->mMax.z = -1e10f;

    // Go through each mesh
    for (unsigned int n = 0; n < scene->mNumMeshes; n++) {
        const aiMesh* mesh = scene->mMeshes[n];

        // Go through each vertex of the mesh
        for (unsigned int t = 0; t < mesh->mNumVertices; t++) {

            // Update min and max based on vertex positions
            aiVector3D tmp = mesh->mVertices[t];

            boundingBox->mMin.x = std::min(boundingBox->mMin.x, tmp.x);
            boundingBox->mMin.y = std::min(boundingBox->mMin.y, tmp.y);
            boundingBox->mMin.z = std::min(boundingBox->mMin.z, tmp.z);

            boundingBox->mMax.x = std::max(boundingBox->mMax.x, tmp.x);
            boundingBox->mMax.y = std::max(boundingBox->mMax.y, tmp.y);
            boundingBox->mMax.z = std::max(boundingBox->mMax.z, tmp.z);
        }
    }
}

void Model::LoadTexture(std::string texturePath, aiTextureType typeName)
{
    fs::path fsPath(texturePath);
    std::string filename = "Assets/"+fsPath.filename().string();
    unsigned int id = Shared::TextureFromFile(texturePath.c_str(), filename);
    auto filepath = fs::current_path().append(filename).string();
    ProjectModals::Texture texture(id, typeName, filepath);
    //Load Texture in GPU. Get the ID.
    // texture.path = texturePath.c_str();

    for(int i = 0;i<meshes.size();i++)
    {
        textureIds.push_back(texture);
    }
}

void Model::loadFromFile(const std::string &filename, Model &model) {

    try
    {
        std::ifstream ifs(filename);
        boost::archive::text_iarchive ia(ifs);
        ia >> model;
    }
    catch(std::exception &e)
    {
        std::cout << "Exception while opening the model file: " << e.what();
    }

    //Load textures to GPU
    //Read from the filenames that need to be loaded
    for (size_t i = 0; i < model.meshes.size(); i++)
    {
        //send the mesh data to GPU. Orginally we manipulated assimp object to load into memory. we now already have the mesh data
        model.meshes[i].setupMesh();
        model.meshes[i].textureIds = &model.textureIds;
    }
    for (size_t i = 0; i < model.textureIds.size(); i++)
    {
        //Just need to generate new textureIds for the texture
        int width, height, nrComponents;
        unsigned char* data = stbi_load(model.textureIds[i].name.c_str(), &width, &height, &nrComponents, 0);
        model.textureIds[i].id = Shared::sendTextureToGPU(data, width, height, nrComponents);
    }
}

void UpdateEngineStateWithFoundTexture(aiTextureType type)
{

}