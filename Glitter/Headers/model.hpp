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

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/access.hpp>
#include <serializer.hpp>
using boost::archive::archive_flags;

unsigned int TextureFromFile(const char* path, std::string filename);
unsigned int sendTextureToGPU(unsigned char* data, int mWidth, int mheight, int nrComponents);

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

    void static loadFromFile(const std::string &filename, Model &model) {

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
            model.textureIds[i].id = sendTextureToGPU(data, width, height, nrComponents);
        }
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


    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & meshes;
        ar & model;
        ar & textureIds;
        ar & directory;
    }
    
};

// BOOST_CLASS_VERSION(Model, 0);