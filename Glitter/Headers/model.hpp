#pragma once
#include <vector>
#include <string>
#include <shader.hpp>
#include <mesh.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"

#include "imgui.h"
#include "ImGuizmo.h"

#include <GLFW/glfw3.h>


unsigned int TextureFromFile(const char* path);
unsigned int sendTextureToGPU(unsigned char* data, int mWidth, int mheight, int nrComponents);

class Model
{
public:
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
            glm::value_ptr(projMatrix), whichTransformActive, ImGuizmo::MODE::WORLD, glm::value_ptr(model));
    }
    std::string getName(){
        return directory;
    }
    std::vector<Mesh>* getMeshes(){
        return &meshes;
    }

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
    
};