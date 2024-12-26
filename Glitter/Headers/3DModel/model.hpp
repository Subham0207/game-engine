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

#include "GLFW/glfw3.h"

#include <map>
#include <serializeAClass.hpp>
#include <Modals/material.hpp>
#include <Lights/cubemap.hpp>
#include <Renderable/renderable.hpp>

class Model: public Renderable
{
public:
    Model()=default;
    Model(std::string path,
    std::map<std::string, BoneInfo>* m_BoneInfoMap = nullptr,
    int* m_BoneCounter = nullptr)
    {
        shader =  new Shader("E:/OpenGL/Glitter/Glitter/Shaders/basic.vert","E:/OpenGL/Glitter/Glitter/Shaders/pbr.frag");
        shader->use();
        loadModel(path, m_BoneInfoMap, m_BoneCounter);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
    }

    void draw() override;

    void bindCubeMapTextures(CubeMap *cubeMap) override;

    void updateModelAndViewPosMatrix(Camera* camera) override;

    void useAttachedShader() override;

    unsigned int getShaderId() const override;

    aiAABB* GetBoundingBox();
    ProjectModals::Texture* LoadTexture(std::string texturePath, aiTextureType typeName) override;

    void imguizmoManipulate(glm::mat4 viewMatrix, glm::mat4 projMatrix) override;

    std::string getName() override{
        return directory;
    }
    std::vector<Mesh>* getMeshes() override{
        return &meshes;
    }

    glm::mat4& getModelMatrix() override{
        return modelMatrix;
    }

    void setModelMatrix(glm::mat4 matrix) override{
        modelMatrix = matrix;
    };

    std::vector<Modals::Material *> getMaterials() override
    {
        return materials;
    }

    void setFileName(std::string filename) override{
        directory = filename;
    }
    
    void static saveSerializedModel(std::string filename, Model &model);

    void static loadFromFile(const std::string &filename, Model &model);

    Shader* shader;
private:
    // model data
    std::vector<Modals::Material*> materials;
    std::vector<ProjectModals::Texture*> textureIds;
    std::vector<Mesh> meshes;
    std::string directory;
    aiAABB* boundingBox;
    glm::mat4 modelMatrix = glm::mat4(1.0f);


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
        ar & modelMatrix;
        ar & materials;
        ar & directory;
    }
    
};

// BOOST_CLASS_VERSION(ModelType, 0);