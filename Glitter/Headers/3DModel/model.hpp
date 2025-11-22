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
#include <Lights/light.hpp>
#include <Serializable.hpp>
#include <functional>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>

namespace Physics {
    class PhysicsObject;
}

class Model: public Renderable, public Serializable
{
public:
    Model()=default;
    Model(std::string path,
    std::map<std::string, BoneInfo>* m_BoneInfoMap = nullptr,
    int* m_BoneCounter = nullptr,
    std::function<void(Assimp::Importer* import, const aiScene*)> onModelComponentsLoad = nullptr);

    void draw(float deltaTime, Camera* camera, Lights* lights, CubeMap* cubeMap) override;

    void bindCubeMapTextures(CubeMap *cubeMap);

    void updateModelAndViewPosMatrix(Camera* camera);

    void useAttachedShader() override;

    aiAABB* GetBoundingBox();
    std::shared_ptr<ProjectModals::Texture> LoadTexture(std::string texturePath, aiTextureType typeName) override;

    void imguizmoManipulate(glm::mat4 viewMatrix, glm::mat4 projMatrix) override;

    std::string getName() override{
        return directory;
    }
    //TODO: Remove this later. As we alrady hav anothr var filename;
    void setDirName(std::string name)
    {
        directory = name;
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

    virtual glm::vec3 GetPosition() override
    {
        return glm::vec3(modelMatrix[3]);
    }

    virtual glm::vec3 GetScale() override
    {
        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(modelMatrix[0])); // X column
        scale.y = glm::length(glm::vec3(modelMatrix[1])); // Y column
        scale.z = glm::length(glm::vec3(modelMatrix[2])); // Z column
        return scale;
    }

    virtual glm::quat GetRot() override
    {
        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(modelMatrix[0]));
        scale.y = glm::length(glm::vec3(modelMatrix[1]));
        scale.z = glm::length(glm::vec3(modelMatrix[2]));

        glm::mat4 rotMat = modelMatrix;

        // Remove scale from rotation matrix
        rotMat[0] /= scale.x;
        rotMat[1] /= scale.y;
        rotMat[2] /= scale.z;

        return glm::quat_cast(rotMat);
    }

    virtual std::string GetGuid() override {
        return getGUID();
    }

    void setTransform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 R = glm::toMat4(rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

        modelMatrix = T * R * S;
    }

    void setTransformFromPhysics(const glm::vec3& position, const glm::quat& rotation)
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 R = glm::toMat4(rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), GetScale());

        modelMatrix = T * R * S;
    }


    std::vector<std::shared_ptr<Modals::Material>> getMaterials() override
    {
        return materials;
    }

    void setFileName(std::string filename) override{
        this->filename = filename;
    }
    
    void static saveSerializedModel(std::string filename, Model &model);

    void static loadFromFile(const std::string &filename, Model &model);

    void attachPhysicsObject(Physics::PhysicsObject* physicsObj);
    void syncTransformationToPhysicsEntity() override;
    void physicsUpdate() override;

    void static initOnGPU(Model* model);

    const std::string contentName() override { return filename;}
    const std::string typeName() const override {return "model";}

    void saveContent(fs::path contentFile, std::ostream& os) override;
    void loadContent(fs::path contentFile, std::istream& is) override;

    Shader* shader;
    std::vector<Mesh> meshes;
    
    std::string filename;
    std::vector<std::shared_ptr<Modals::Material>> materials;
    std::vector<std::shared_ptr<ProjectModals::Texture>> textureIds;
private:
    std::string directory;
    aiAABB* boundingBox;
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    Physics::PhysicsObject* physicsObject = NULL;
    std::string physicsBodyType = "";


    void loadModel(std::string path,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter,
    std::function<void(Assimp::Importer* import, const aiScene*)> onModelComponentsLoad = nullptr);

    void processNode(
    aiNode* node,
    const aiScene* scene,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter);

    std::shared_ptr<ProjectModals::Texture> processEmbeddedTexture(const aiScene* scene, aiMaterial* material, aiTextureType type);
    
    Mesh processMesh(
    aiMesh* mesh,
    const aiScene* scene,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter);

    void loadMaterialTextures(aiMaterial* mat, aiTextureType type);

    std::shared_ptr<ProjectModals::Texture> loadEmbeddedTexture(const aiTexture* texture, aiTextureType textureType);

    void calculateBoundingBox(const aiScene* scene);

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & meshes;
        ar & modelMatrix;
        ar & materials;
        ar & textureIds;
        ar & directory;
        ar & physicsBodyType;
    }
    
};

// BOOST_CLASS_VERSION(ModelType, 0);