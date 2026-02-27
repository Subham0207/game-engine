#pragma once
#include "3DModel/model.hpp"
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <filesystem>
#include "Helpers/AssimpHelpers.hpp"
#include <Helpers/vertexBoneDataHelper.hpp>
#include "Helpers/Shared.hpp"
#include <Modals/vertex.hpp>
#include <stb_image.h>
#include <stb_image_write.h>
#include <EngineState.hpp>
#include "ImGuizmo.h"
#include <Physics/box.hpp>
#include <Modals/3DModelType.hpp>

#include "Materials/MaterialInstance.hpp"
#include "Materials/Material.hpp"
#include "Materials/TextureUnits.hpp"

// using namespace std;
namespace fs = std::filesystem;

void Model::loadModel(
    std::string path,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter,
    std::function<void(Assimp::Importer* import, const aiScene*)> onModelComponentsLoad)
{
    directory = path.substr(0, path.find_last_of('/'));
    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);

    std::shared_ptr<Materials::Material> material;

    if(m_BoneInfoMap && m_BoneCounter)
    {
        const auto vertPath = (engineFSPath / "Shaders/basic.vert").string();
        const auto fragPath = (engineFSPath / "Shaders/pbr.frag").string();
        material = std::make_shared<Materials::Material>(directory, vertPath, fragPath);
    }
    else
    {
        auto vertPath = (engineFSPath / "Shaders/staticShader.vert").string();
        auto fragPath = (engineFSPath / "Shaders/staticShader.frag").string();
        material = std::make_shared<Materials::Material>(directory, vertPath, fragPath);
    }
    //material->Bind(); // But the texture units are not initialized yet.
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }


    processNode(scene->mRootNode, scene, m_BoneInfoMap, m_BoneCounter, material);

    if(onModelComponentsLoad)
    onModelComponentsLoad(&import, scene);
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

    for (auto mesh : model.meshes)
    {
        mesh.mMaterial->save(dir);
    }

    //Texture-ids needs to  generated again
    //They will need to bound again to GPU
}

std::shared_ptr<ProjectModals::Texture> Model::processEmbeddedTexture(const aiScene* scene, aiMaterial* material, aiTextureType type)
{
    aiString texturePath;
    if (material->GetTexture(type, 0, &texturePath) == AI_SUCCESS) {
        std::cout << "Material has texture type " << type << ": " << texturePath.C_Str() << std::endl;

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
                    return loadEmbeddedTexture(embeddedTexture, aiTextureType_DIFFUSE_ROUGHNESS);
                return loadEmbeddedTexture(embeddedTexture, type);
                } else {
                    std::cout << "Uncompressed texture" << std::endl;
                    std::cout << "Dimensions: " << embeddedTexture->mWidth << " x " << embeddedTexture->mHeight << std::endl;
                }
            }
        }
    }
    return {nullptr};
}

std::shared_ptr<ProjectModals::Texture> Model::loadEmbeddedTexture(const aiTexture* texture, aiTextureType textureType)
{
    auto currentProjectLocation = fs::path(EngineState::state->currentActiveProjectDirectory);
    auto filename = (currentProjectLocation / "Assets" / fs::path(texture->mFilename.C_Str()).filename().string()).string();
    if(textureIds.size() > 0)
    {
        for (size_t i = 0; i < textureIds.size(); i++)
        {
            if(textureIds[i]->name == texture->mFilename.C_Str())
            return textureIds[i];
        }
        
    }
    

    int mWidth, mheight, nrComponents;
    unsigned char* data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(texture->pcData), texture->mWidth, &mWidth, &mheight, &nrComponents, 0);

    if (!data) {
        // stbi_failure_reason() gives the reason why it failed.
        std::string error_msg = "Failed to load embedded texture: ";
        error_msg += stbi_failure_reason();
        
        // Throw a standard exception that your try-catch block is expecting
        throw std::runtime_error(error_msg); 
    }
    
    //TODO: Saving logic should be removed from here and should be saved when material it is attached to is saved ??
    stbi_write_png(filename.c_str(), mWidth, mheight, nrComponents, data, 0);
    unsigned int textureID = Shared::sendTextureToGPU(data, mWidth, mheight, nrComponents);
    auto filepath = fs::current_path().append(filename).string();
    auto newTexture = std::make_shared<ProjectModals::Texture>(textureID, textureType, filepath);
    textureIds.push_back(newTexture);
    return textureIds[textureIds.size() - 1];
}

void Model::processNode(
    aiNode* node,
     const aiScene* scene,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter,
    std::shared_ptr<Materials::Material> material)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        // Store the information about a mesh
        // that we can comprehend right now and push into an array
        auto materialInstance = std::make_shared<Materials::MaterialInstance>(directory + std::to_string(i), material);
        meshes.push_back(processMesh(mesh, scene, m_BoneInfoMap, m_BoneCounter, materialInstance));
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, m_BoneInfoMap, m_BoneCounter, material);
    }
}

Mesh Model::processMesh(
    aiMesh* mesh,
    const aiScene* scene,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter,
    std::shared_ptr<Materials::MaterialInstance> materialInstance)
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

    vertex.animatedPos = vertex.Position;

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

    if (m_BoneCounter && m_BoneInfoMap)
    {
        Helpers::ExtractBoneWeightForVertices(vertices,mesh,scene, *m_BoneInfoMap, *m_BoneCounter);
    }

    auto justMesh = Mesh(vertices, indices, materialInstance);

    //3. process materials
    // What if there are no textures found there is no reason to error out on that case
    // Sample color instead so that will be vertex color
    const std::vector<aiTextureType> textureTypes = {
        aiTextureType_DIFFUSE,
        aiTextureType_SPECULAR,
        aiTextureType_NORMALS,
        aiTextureType_DIFFUSE_ROUGHNESS,
        aiTextureType_AMBIENT_OCCLUSION,
        aiTextureType_METALNESS
    };

    aiMaterial* ai_material = scene->mMaterials[mesh->mMaterialIndex];
    // if (justMesh.material) {
    //     justMesh.material.reset();
    // }
    try
    {
        auto& textureUnits = justMesh.mMaterial->GetTextureUnits();
        for (aiTextureType type : textureTypes) {
            if (!justMesh.mMaterial) {
                break; 
            } 
            switch (type)
            {
                case aiTextureType_DIFFUSE:
                    textureUnits.albedo = processEmbeddedTexture(scene, ai_material, type);
                    break;
                case aiTextureType_SPECULAR:
                    textureUnits.roughness = processEmbeddedTexture(scene, ai_material, type);
                    break;
                case aiTextureType_NORMALS:
                    textureUnits.normal = processEmbeddedTexture(scene, ai_material, type);
                    break;
                case aiTextureType_DIFFUSE_ROUGHNESS:
                    textureUnits.roughness = processEmbeddedTexture(scene, ai_material, type);
                    break;
                case aiTextureType_AMBIENT_OCCLUSION:
                    textureUnits.ao = processEmbeddedTexture(scene, ai_material, type);
                    break;
                case aiTextureType_METALNESS:
                    textureUnits.metalness = processEmbeddedTexture(scene, ai_material, type);
                    break;
                default:
                    break;
            }
        }
    
        materials.push_back(justMesh.mMaterial);

    }catch(std::exception &e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return justMesh;
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

void Model::drawGeometryOnly(float deltaTime)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].DrawOnlyGeometry();
}

std::vector<unsigned int> Model::GetIndices()
{
    int baseVert = 0;
    std::vector<unsigned int>   tris;
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        auto meshIndices = meshes[i].GetIndices();
        auto meshVerts = meshes[i].GetWorldVertices();
        for (size_t j = 0; j < meshIndices.size(); ++j)
        {
            tris.push_back(baseVert + (int)meshIndices[j]);
        }

        baseVert += (int)meshVerts.size();
    }

    return tris;
}

std::vector<ProjectModals::Vertex> Model::GetWorldVertices()
{
   std::vector<ProjectModals::Vertex> result;
    for (const auto& mesh : meshes)
    {
        const auto& localVerts = mesh.vertices; // your original local array
        for (const auto& v : localVerts)
        {
            ProjectModals::Vertex wv = v;
            glm::vec4 p = modelMatrix * glm::vec4(v.Position, 1.0f);
            wv.Position = glm::vec3(p);
            result.push_back(wv);
        }
    }

    return result;
}

void Model::BuildFlattenedGeometry(std::vector<ProjectModals::Vertex>& outVerts,
                                std::vector<unsigned int>& outIndices)
{
    outVerts.clear();
    outIndices.clear();

    unsigned int baseVert = 0;

    for (const auto& mesh : meshes)
    {
        const auto& localVerts   = mesh.vertices;     // single source of truth
        const auto& meshIndices  = mesh.indices; // local indices

        // Push world-space verts for this mesh
        for (const auto& v : localVerts)
        {
            ProjectModals::Vertex wv = v;
            glm::vec4 p = modelMatrix * glm::vec4(v.Position, 1.0f);
            wv.Position = glm::vec3(p);
            outVerts.push_back(wv);
        }

        // Push indices with correct offset
        for (size_t j = 0; j < meshIndices.size(); ++j)
        {
            outIndices.push_back(baseVert + meshIndices[j]);
        }

        baseVert += static_cast<unsigned int>(localVerts.size());
    }
}

void Model::draw(float deltaTime, Camera *camera, Lights *lights, CubeMap *cubeMap)
{
    bindCubeMapTextures(cubeMap);

	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(camera, lights, modeltype, modelMatrix);

    if(physicsObject)
    {
        if(EngineState::state->isPlay)
        {
            this->setTransformFromPhysics(physicsObject->model->GetPosition(), physicsObject->model->GetRot());
        }
        else
        {
            auto pos = GetPosition();
            auto rot = GetRot();
            physicsObject->model->setTransformFromPhysics(pos, rot);
        }

    }
}

void Model::bindCubeMapTextures(CubeMap *cubeMap)
{
    glActiveTexture(GL_TEXTURE0+6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap->irradianceMap);
    glActiveTexture(GL_TEXTURE0+7);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap->prefilterMap);
    glActiveTexture(GL_TEXTURE0+8);
    glBindTexture(GL_TEXTURE_2D, cubeMap->brdfLUTTexture);
}

aiAABB *Model::GetBoundingBox()
{
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

std::shared_ptr<ProjectModals::Texture> Model::LoadTexture(std::string texturePath, aiTextureType typeName)
{
    fs::path fsPath(texturePath);
    auto currentProjectLocation = fs::path(EngineState::state->currentActiveProjectDirectory);
    auto filename = (currentProjectLocation / "Assets" / fsPath.filename().string()).string();
    unsigned int id = Shared::TextureFromFile(texturePath.c_str(), filename);
    auto filepath = fs::current_path().append(filename).string();
    auto texture = std::make_shared<ProjectModals::Texture>(id, typeName, filepath);
    //Load Texture in GPU. Get the ID.
    // texture.path = texturePath.c_str();

    textureIds.push_back(texture);

    //Update the material and since material are reference types; The correct mesh should get updated
    return texture;
}

void Model::imguizmoManipulate(glm::mat4 viewMatrix, glm::mat4 projMatrix)
{
    ImGuizmo::Manipulate(
    glm::value_ptr(viewMatrix),
    glm::value_ptr(projMatrix), getUIState().whichTransformActive, ImGuizmo::MODE::LOCAL, glm::value_ptr(modelMatrix));
}
void Model::loadFromFile(const std::string &filename, Model &model, std::shared_ptr<Materials::Material>& material)
{

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

    Model::initOnGPU(&model, material);
}

void UpdateEngineStateWithFoundTexture(aiTextureType type)
{

}

void Model::attachPhysicsObject(Physics::PhysicsObject* physicsObj)
{
    this->physicsBodyType = "Box";
    this->physicsObject = physicsObj;
}

void Model::syncTransformationToPhysicsEntity()
{
    if(this->physicsObject)
    {
        physicsObject->model->setModelMatrix(getModelMatrix());
        physicsObject->syncTransformation();
    }
}

void Model::physicsUpdate()
{
    if(this->physicsObject)
    physicsObject->PhysicsUpdate();
}

void Model::LoadA3DModel(
    const aiScene* scene,
    bool isSkinned,
    const std::string& path,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter
    )
{
    filename = fs::path(path).filename().string();
    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    modeltype = ModelType::ACTUAL_MODEL;
    if(isSkinned)
    {
        auto vertPath = (engineFSPath / "Shaders/basic.vert").string();
        auto fragPath = (engineFSPath / "Shaders/pbr.frag").string();
        auto material = std::make_shared<Materials::Material>(filename, vertPath, fragPath);
        processNode(scene->mRootNode, scene, m_BoneInfoMap, m_BoneCounter, material);
    }
    else
    {
        auto vertPath = (engineFSPath / "Shaders/staticShader.vert").string();
        auto fragPath = (engineFSPath / "Shaders/staticShader.frag").string();
        auto material = std::make_shared<Materials::Material>(filename, vertPath, fragPath);
        processNode(scene->mRootNode, scene, nullptr, nullptr, material);
    }


    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));

}

Model::Model(std::string path,
std::map<std::string, BoneInfo>* m_BoneInfoMap,
int* m_BoneCounter,
std::function<void(Assimp::Importer* import, const aiScene*)> onModelComponentsLoad)
: Serializable()
{
    modeltype = ModelType::ACTUAL_MODEL;
    loadModel(path, m_BoneInfoMap, m_BoneCounter, std::move(onModelComponentsLoad));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));

    filename = fs::path(path).filename().string();
}

void Model::saveContent(fs::path contentFile, std::ostream& os)
{
    // guid();
    Model::saveSerializedModel(contentFile.string(), *this);
}

void Model::loadContent(fs::path contentFile, std::istream& is)
{
    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    auto vertPath = engineFSPath / "Shaders/staticShader.vert";
    auto fragPath = engineFSPath / "Shaders/staticShader.frag";
    auto material = std::make_shared<Materials::Material>("Material",vertPath.string(), fragPath.string());

    modeltype = ModelType::ACTUAL_MODEL;
    Model::loadFromFile(contentFile.string(), *this, material);


    if(!physicsBodyType.empty())
        this->attachPhysicsObject(new Physics::Box(&getPhysicsSystem(), false, true));
}

void Model::initOnGPU(Model* model, std::shared_ptr<Materials::Material>& material)
{
    //Load textures to GPU
    //Read from the filenames that need to be loaded
    for (size_t i = 0; i < model->meshes.size(); i++)
    {
        //send the mesh data to GPU. Orginally we manipulated assimp object to load into memory. we now already have the mesh data
        model->meshes[i].setupMesh();
        model->meshes[i].setupMaterial();

        //Attach correct texture to each meshes;
        //model.meshes[i].albedo.Filename = model.textureIds.findTexture(model.meshes[i].albedo.Filename)
    }
}

bool Model::ShouldRender() {
    if(modeltype == ModelType::ACTUAL_MODEL)
    {
        return true;
    }

    if(!EngineState::state->isPlay && modeltype != ModelType::ACTUAL_MODEL)
    {
        return true;
    }
    return false;
}