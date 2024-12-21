#pragma once
#include <vector>
#include <string>
#include "Helpers/shader.hpp"
#include "mesh.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "3DModel/Skeleton/AnimData.hpp"

#include "imgui.h"
#include "ImGuizmo.h"

#include "GLFW/glfw3.h"

#include <map>
#include <serializeAClass.hpp>
#include <Modals/material.hpp>

class Model
{
public:
    Model()=default;
    Model(std::string path,
    std::map<std::string, BoneInfo>* m_BoneInfoMap = nullptr,
    int* m_BoneCounter = nullptr)
    {
        loadModel(path, m_BoneInfoMap, m_BoneCounter);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    }
    void Draw(Shader* shader, GLFWwindow* window);
    aiAABB* GetBoundingBox();
    ProjectModals::Texture* LoadTexture(std::string texturePath, aiTextureType typeName);
    glm::mat4 model = glm::mat4(1.0f);

    void imguizmoManipulate(glm::mat4 viewMatrix, glm::mat4 projMatrix)
    {
            ImGuizmo::Manipulate(
            glm::value_ptr(viewMatrix),
            glm::value_ptr(projMatrix), whichTransformActive, ImGuizmo::MODE::LOCAL, glm::value_ptr(model));
    }
    std::string getName(){
        return directory;
    }
    std::vector<Mesh>* getMeshes(){
        return &meshes;
    }

    auto getMaterials()
    {
        return materials;
    }
    
    void static saveSerializedModel(std::string filename,  Model &model);

    void static loadFromFile(const std::string &filename, Model &model);

private:
    // model data
    std::vector<Modals::Material*> materials;
    std::vector<ProjectModals::Texture*> textureIds;
    std::vector<Mesh> meshes;
    std::string directory;
    aiAABB* boundingBox;

    ImGuizmo::OPERATION whichTransformActive = ImGuizmo::OPERATION::TRANSLATE;

    void loadModel(std::string path,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter);
    void processNode(
    aiNode* node,
    const aiScene* scene,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter);
    ProjectModals::Texture* processEmbeddedTexture(const aiScene* scene, aiMaterial* material, aiTextureType type);
    Mesh processMesh(
    aiMesh* mesh,
    const aiScene* scene,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter);
    void loadMaterialTextures(aiMaterial* mat, aiTextureType type);
    ProjectModals::Texture* loadEmbeddedTexture(const aiTexture* texture, aiTextureType textureType);
    void calculateBoundingBox(const aiScene* scene);

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & meshes;
        ar & model;
        ar & materials;
        ar & directory;
    }
    
};

// BOOST_CLASS_VERSION(ModelType, 0);