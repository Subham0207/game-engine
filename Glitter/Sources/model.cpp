#include "model.hpp"
#include <iostream>
#include <stb_image.h>

void Model::loadModel(std::string path)
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

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        // Store the information about a mesh
        // that we can comprehend right now and push into an array
        meshes.push_back(processMesh(mesh, scene));
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
   std::vector<Vertex> vertices;
   std::vector<unsigned int>indices;
   std::vector<Texture> textures;

    //1. Procesing vertices
   for(unsigned int i = 0; i < mesh->mNumVertices; i++)
   {
    Vertex vertex;
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
        std::cout << "Vertex Color set to red" << std::endl;
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

    //There is a chance the texture might not be provided with the model and we need to exclusively provide them.
    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    std::cout << "textures size " << textures.size() << std::endl;

    return Mesh(vertices, indices, textures);
}

// std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
// {
//     std::vector<Texture> textures;
//     for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
//     {
//         aiString str;
//         mat->GetTexture(type, i, &str);
//         // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
//         bool skip = false;
//         for (unsigned int j = 0; j < textures_loaded.size(); j++)
//         {
//             if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
//             {
//                 textures.push_back(textures_loaded[j]);
//                 skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
//                 break;
//             }
//         }
//         if (!skip)
//         {   // if texture hasn't been loaded already, load it
//             Texture texture;
//             texture.id = TextureFromFile(str.C_Str(), this->directory);
//             texture.type = typeName;
//             texture.path = str.C_Str();
//             textures.push_back(texture);
//             textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
//         }
//     }
//     return textures;
// }

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    //How do we deal with textures from different type of format - Like FBX can have embedded texture
    std::cout << "Number of " << type << " " << mat->GetTextureCount(type) << std::endl; 
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        if(AI_SUCCESS != mat->GetTexture(type, i, &str))
        {
            std::cout << "Cannot load texture of type" << type << std::endl; 
        }
    }
    return textures;
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

void Model::Draw(Shader* shader)
{
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

void Model::LoadTexture(std::string texturePath, std::string typeName)
{
    Texture texture;
    texture.id = TextureFromFile(texturePath.c_str(), this->directory);
    texture.type = typeName;
    texture.path = texturePath.c_str();

    for(int i = 0;i<meshes.size();i++)
    {
        meshes[i].textures.push_back(texture);
    }
}


unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}