#pragma once
#include <algorithm>
#include <vector>
#include <string>
#include "Helpers/shader.hpp"
#include "mesh.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "3DModel/Skeleton/AnimData.hpp"
#include "GLFW/glfw3.h"
#include <map>
#include <Materials/IMaterial.hpp>
#include <Lights/cubemap.hpp>
#include <Renderable/renderable.hpp>
#include <Lights/light.hpp>
#include <Serializable.hpp>
#include <functional>
#include <optional>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <unordered_set>

#include "Materials/Material.hpp"
#include "Modals/texture.hpp"
#include "Physics/PhysicsBodySettings.hpp"
#include "serializer.hpp"

enum ModelType;

namespace Physics {
    class PhysicsObject;
}

class Model: public Renderable, public Serializable
{
public:
    Model()=default;
    Model(
        std::string path,
        std::string engineAssetsFolder,
        std::map<std::string, BoneInfo>* m_BoneInfoMap = nullptr,
        int* m_BoneCounter = nullptr,
        std::function<void(Assimp::Importer* import, const aiScene*)> onModelComponentsLoad = nullptr);

    std::string GetClassId() const override { return "Model"; }

    virtual void onTick(float deltaTime){};
    virtual void onStart(){};
    virtual void onDestroy(){};

    //Only Applies to Physics body of Kinematics Motion type. Call it inside tick
    void MoveBody(const glm::vec3& position, float deltaTime) const;
    void MoveBody(const glm::vec3& position, const glm::quat& rotation, float deltaTime) const;

    // This can load 3d model file example: warrior.fbx;
    void LoadA3DModel(
        const aiScene* scene,
        bool isSkinned,
        const std::string& path,
        std::map<std::string, BoneInfo>* m_BoneInfoMap,
        int* m_BoneCounter);

    void static processingVerticesForAMesh(
        aiMesh* mesh,
        std::vector<ProjectModals::Vertex> &vertices,
        std::vector<unsigned int> &indices){};

    void draw(float deltaTime, Camera* camera, Lights* lights, CubeMap* cubeMap, const std::vector<glm::mat4>* finalBoneMatrix);
    void drawGeometryOnly(float deltaTime) override;

    virtual std::vector<ProjectModals::Vertex> GetWorldVertices() override;
    virtual std::vector<unsigned int> GetIndices() override;
    void Model::BuildFlattenedGeometry(std::vector<ProjectModals::Vertex>& outVerts,
                                   std::vector<unsigned int>& outIndices);
    virtual ModelType getModelType() override {return modeltype;}

    void bindCubeMapTextures(CubeMap *cubeMap);

    bool ShouldRender() override;

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
        return getAssetId();
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


    std::vector<std::shared_ptr<Materials::IMaterial>> getMaterials() override
    {
        return materials;
    }

    void setFileName(std::string filename) override{
        this->filename = filename;
    }
    
    void static saveSerializedModel(std::string filename, Model &model);

    void static loadFromFile(const std::string &filename, Model &model, std::shared_ptr<Materials::Material>& material);
    static std::shared_ptr<Model> loadWithClassFactory(const fs::path& assetRoot, const std::string& filename);

    void attachPhysicsObject(Physics::PhysicsObject* physicsObj);
    [[nodiscard]] bool hasPhysicsObject() const { return physicsObject != nullptr; }
    void clearPhysicsObject();

    void setPhysicsBodySettings(const std::optional<Physics::PhysicsBodySettings>& settings);
    [[nodiscard]] const std::optional<Physics::PhysicsBodySettings>& getPhysicsBodySettings() const { return physicsBodySettings; }
    bool setCustomColliderGeometryFromFile(const std::string& colliderAssetPath, std::string* outError = nullptr);

    void setGameplayTags(const std::vector<std::string>& tags)
    {
        gameplayTags = tags;
        gameplayTagSet.clear();
        gameplayTagSet.insert(gameplayTags.begin(), gameplayTags.end());
    }
    [[nodiscard]] const std::vector<std::string>& getGameplayTags() const { return gameplayTags; }
    [[nodiscard]] const std::unordered_set<std::string>& GetGameplayTags() const override { return gameplayTagSet; }
    [[nodiscard]] bool hasGameplayTag(const std::string& tag) const
    {
        return gameplayTagSet.find(tag) != gameplayTagSet.end();
    }
    [[nodiscard]] std::vector<Renderable*> GetOverlappingSensors() const override;

    void ensureStaticBoxCollider();
    void syncPhysicsColliderToModelTransform();
    [[nodiscard]] glm::vec3 computeLocalMeshHalfExtents() const;

    void static initOnGPU(Model* model, std::shared_ptr<Materials::Material>& material);

    const std::string contentName() override { return filename;}
    const std::string typeName() const override {return "model";}

    void saveContent(fs::path contentFile, std::ostream& os) override;
    void loadContent(fs::path contentFile, std::istream& is) override;
    virtual void setIsSelected(bool isSelected) override
    {
        this->isSelected = isSelected;
    };
    virtual bool getIsSelected() override
    {
        return isSelected;
    };

    ModelType modeltype;
    bool isSelected = false;

    std::vector<Mesh> meshes;
    
    std::string classId = "None";
    std::string filename;
    std::vector<std::shared_ptr<Materials::IMaterial>> materials;
    std::vector<std::shared_ptr<ProjectModals::Texture>> textureIds;
private:
    bool started = false;
    std::string directory;
    aiAABB* boundingBox;
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    Physics::PhysicsObject* physicsObject = NULL;
    std::optional<Physics::PhysicsBodySettings> physicsBodySettings = std::nullopt;
    std::vector<std::string> gameplayTags;
    std::unordered_set<std::string> gameplayTagSet;


    void loadModel(std::string path,
    std::string engineAssetsFolder,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter,
    std::function<void(Assimp::Importer* import, const aiScene*)> onModelComponentsLoad = nullptr);

    void processNode(
    aiNode* node,
    const aiScene* scene,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter,
    std::shared_ptr<Materials::Material> material);

    std::shared_ptr<ProjectModals::Texture> processEmbeddedTexture(const aiScene* scene, aiMaterial* material, aiTextureType type);
    
    Mesh processMesh(
    aiMesh* mesh,
    const aiScene* scene,
    std::map<std::string, BoneInfo>* m_BoneInfoMap,
    int* m_BoneCounter,
    std::shared_ptr<Materials::MaterialInstance> materialInstance);

    void loadMaterialTextures(aiMaterial* mat, aiTextureType type);

    std::shared_ptr<ProjectModals::Texture> loadEmbeddedTexture(const aiTexture* texture, aiTextureType textureType);

    void calculateBoundingBox(const aiScene* scene);
    void rebuildPhysicsObjectFromSettings();
    void syncGameplayTagSet();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        static_assert(sizeof(meshes) > 0, "Mesh size check");
        ar & meshes;
        ar & modelMatrix;
        ar & directory;
        ar & physicsBodySettings;
        ar & gameplayTags;
        if (version >= 1)
        {
            ar & classId;
        }
        else if (Archive::is_loading::value)
        {
            classId = "None";
        }
    }
    
};

BOOST_CLASS_VERSION(Model, 1);