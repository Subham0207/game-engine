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
    if(textureIds->size() == 0)
    {
        shader->setBool("useTexture", false);
    }
    else{
    shader->setBool("useTexture", true);
    }
    
    unsigned int diffuseNr = 0;
    unsigned int specularNr = 0;
    unsigned int normalNr = 0;    
    std::cout << "---START---" << std::endl;
    
    const std::vector<aiTextureType> textureTypes = {
    aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_NORMALS
    };

    for (unsigned int i = 0; i < 3; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        glBindTexture(GL_TEXTURE_2D, 0);
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        aiTextureType name = textureTypes.at(i);
        if (name == aiTextureType_DIFFUSE)
        {
            diffuseNr++;
            std::cout << "Diffuse" << std::endl;
            if (i >= 0 && i < textureIds->size())
            {
                shader->setInt("material.diffuse", i);
            }
        }
        else if (name == aiTextureType_SPECULAR)
        {
            specularNr++;
            std::cout << "Specular" << std::endl;
            if (i >= 0 && i < textureIds->size())
            {
                shader->setInt("material.specular", i);
            }
        }
        else if (name == aiTextureType_NORMALS)
        {
            normalNr++;
            std::cout << "Normals" << std::endl;
            if (i >= 0 && i < textureIds->size())
            {
                shader->setInt("material.normal", i);
                shader->setBool("useNormalMap", true);
            }
            else
            {
                shader->setBool("useNormalMap", false);
            }
        }
        
        if (i >= 0 && i < textureIds->size())
        {
            glBindTexture(GL_TEXTURE_2D, textureIds->at(i).id);
        }
    }

    std::cout << "---END---" << std::endl;
    glActiveTexture(GL_TEXTURE0);

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
