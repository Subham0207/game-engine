#include "3DModel/mesh.hpp"

Mesh::Mesh(std::vector<ProjectModals::Vertex> vertices, std::vector<unsigned int> indices)
{
    this->vertices = vertices;
    this->indices = indices;

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

    const std::vector<aiTextureType> textureTypes = {
        aiTextureType_DIFFUSE, aiTextureType_NORMALS, aiTextureType_METALNESS,
        aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_AMBIENT_OCCLUSION
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    for(unsigned int i = 0;i < textureTypes.size(); i++)
    {
        switch (textureTypes[i])
        {
            case aiTextureType_DIFFUSE:
            {
                if(material->albedo != nullptr)
                {
                    glActiveTexture(GL_TEXTURE0 + 1);
                    glBindTexture(GL_TEXTURE_2D, material->albedo->id);
                }
                break;
            }
            case aiTextureType_NORMALS:
            {
                if(material->normal != nullptr)
                {
                    glActiveTexture(GL_TEXTURE0 + 2);
                    glBindTexture(GL_TEXTURE_2D, material->normal->id);
                }
                break;
            }
            case aiTextureType_METALNESS:
            {
                if(material->metalness != nullptr)
                {
                    glActiveTexture(GL_TEXTURE0 + 3);
                    glBindTexture(GL_TEXTURE_2D, material->metalness->id);
                }
                break;
            }
            case aiTextureType_DIFFUSE_ROUGHNESS:
            {
                if(material->roughness != nullptr)
                {
                    glActiveTexture(GL_TEXTURE0 + 4);
                    glBindTexture(GL_TEXTURE_2D, material->roughness->id);
                }
                break;
            }
            case aiTextureType_AMBIENT_OCCLUSION:
            {
                if(material->ao != nullptr)
                {
                    glActiveTexture(GL_TEXTURE0 + 5);
                    glBindTexture(GL_TEXTURE_2D, material->ao->id);
                }
                break;
            }
            default:
                break;
        }
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

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ProjectModals::Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ProjectModals::Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ProjectModals::Vertex), (void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ProjectModals::Vertex), (void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::TexCoords));
    //Vertex Color
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ProjectModals::Vertex), (void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::Color));

    //TODO: maybe we are missing tangents and bitangents binding here...

    // ids
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 4, GL_INT, sizeof(ProjectModals::Vertex), (void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::m_BoneIDs));

    // weights
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(ProjectModals::Vertex),(void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::m_Weights));   

    glBindVertexArray(0);
}
