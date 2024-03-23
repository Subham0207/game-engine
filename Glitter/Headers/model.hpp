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

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

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
    void LoadTexture(std::string texturePath, std::string typeName);
    glm::mat4 model = glm::mat4(1.0f);

    double getX(){return model[3][0];}
    double getY(){return model[3][1];}
    double getZ(){return model[3][2];}

    double getScaleX(){return model[0][0];}
    double getScaleY(){return model[1][1];}
    double getScaleZ(){return model[2][2];}



    void imguizmoManipulate(glm::mat4 viewMatrix, glm::mat4 projMatrix)
    {
            ImGuizmo::Manipulate(
            glm::value_ptr(viewMatrix),
            glm::value_ptr(projMatrix), whichTransformActive, ImGuizmo::MODE::WORLD, glm::value_ptr(model));
    }
    std::string getName(){
        return directory;
    }
private:
    // model data
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;
    aiAABB* boundingBox;

    ImGuizmo::OPERATION whichTransformActive = ImGuizmo::OPERATION::TRANSLATE;

    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
        std::string typeName);
    void Model::calculateBoundingBox(const aiScene* scene);
    
};