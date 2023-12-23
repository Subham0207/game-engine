#include "model.hpp"
#include <iostream>

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
    vertex.TexCoords = glm::vec2(0.0f, 0.0f);

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

    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    return Mesh(vertices, indices, textures);
}

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
