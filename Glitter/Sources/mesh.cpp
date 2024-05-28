#include "mesh.hpp"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> *textureIds)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textureIds = textureIds;

    setupMesh();
}

void Mesh::Draw(Shader* shader)
{
    // if(textureIds->size() == 0)
    // {
    //     shader->setBool("useTexture", false);
    // }
    // else{
    // shader->setBool("useTexture", true);
    // }
    
    for (unsigned int i = 0; i < textureIds->size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        glBindTexture(GL_TEXTURE_2D, 0);
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        aiTextureType name = textureIds->at(i).type;
        if (name == aiTextureType_DIFFUSE)// albedoMap
        {
            shader->setInt("albedoMap", i);
        }
        else if (name == aiTextureType_NORMALS)//normalMap
        {
            shader->setInt("normalMap", i);
        }
        else if (name == aiTextureType_METALNESS)//metallicMap
        {
            shader->setInt("metallicMap", i);
        }
        else if (name == aiTextureType_DIFFUSE_ROUGHNESS)//roughnessMap
        {
            shader->setInt("roughnessMap", i);
        }
        else if (name == aiTextureType_AMBIENT_OCCLUSION)//AO
        {
            shader->setInt("aoMap", i);
        }
        glBindTexture(GL_TEXTURE_2D, textureIds->at(i).id);
    }
    // I don't need to active texture 0 again ?? Why was I doing this ?
    // glActiveTexture(GL_TEXTURE0);

    // draw meshs
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    //Vertex Color
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));

    glBindVertexArray(0);
}
