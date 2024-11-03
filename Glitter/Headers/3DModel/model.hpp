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

#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/access.hpp"
#include "serializer.hpp"
#include <map>
using boost::archive::archive_flags;

#define MAX_BONE_WEIGHTS 100

class Model
{
public:
    Model()=default;
    Model(char* path)
    {
        loadModel(path);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    }
    void Draw(Shader* shader, GLFWwindow* window);
    aiAABB* GetBoundingBox();
    void LoadTexture(std::string texturePath, aiTextureType typeName);
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
    
    void static saveSerializedModel(std::string filename,  Model &model);

    void static loadFromFile(const std::string &filename, Model &model);

    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }    

private:
    // model data
    std::vector<Texture> textureIds;
    std::vector<aiTextureType> textureTypes;
    std::vector<Mesh> meshes;
    std::string directory;
    aiAABB* boundingBox;

    ImGuizmo::OPERATION whichTransformActive = ImGuizmo::OPERATION::TRANSLATE;

    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    void processTexture(const aiScene *scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    void loadMaterialTextures(aiMaterial* mat, aiTextureType type);
    void loadEmbeddedTexture(const aiTexture* texture, aiTextureType textureType);
    void Model::calculateBoundingBox(const aiScene* scene);
    void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);

    std::map<std::string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;


    void SetVertexBoneDataToDefault(Vertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_WEIGHTS; i++)
        {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }


    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & meshes;
        ar & model;
        ar & textureIds;
        ar & directory;
    }
    
};

// BOOST_CLASS_VERSION(ModelType, 0);