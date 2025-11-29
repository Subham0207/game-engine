#include "3DModel/mesh.hpp"
#include <EngineState.hpp>

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
                glActiveTexture(GL_TEXTURE0 + 1);
                if(material &&  material->albedo)
                {
                    glBindTexture(GL_TEXTURE_2D, material->albedo->id);
                }
                else{
                    glBindTexture(GL_TEXTURE_2D, getUIState().whiteAOTextureID);                    
                }
                break;
            }
            case aiTextureType_NORMALS:
            {
                glActiveTexture(GL_TEXTURE0 + 2);
                if(material &&  material->normal)
                {
                    glBindTexture(GL_TEXTURE_2D, material->normal->id);
                }
                else{
                    glBindTexture(GL_TEXTURE_2D, getUIState().flatNormalTextureID); 
                }
                break;
            }
            case aiTextureType_METALNESS:
            {
                glActiveTexture(GL_TEXTURE0 + 3);
                if(material &&  material->metalness)
                {
                    glBindTexture(GL_TEXTURE_2D, material->metalness->id);
                }
                else{
                    glBindTexture(GL_TEXTURE_2D, getUIState().whiteAOTextureID);                    
                }
                break;
            }
            case aiTextureType_DIFFUSE_ROUGHNESS:
            {
                glActiveTexture(GL_TEXTURE0 + 4);
                if(material &&  material->roughness)
                {
                    glBindTexture(GL_TEXTURE_2D, material->roughness->id);
                }
                else{
                    glBindTexture(GL_TEXTURE_2D, getUIState().whiteAOTextureID);                    
                }
                break;
            }
            case aiTextureType_AMBIENT_OCCLUSION:
            {
                glActiveTexture(GL_TEXTURE0 + 5);
                if(material &&  material->ao)
                {
                    glBindTexture(GL_TEXTURE_2D, material->ao->id);
                }
                else{
                    glBindTexture(GL_TEXTURE_2D, getUIState().whiteAOTextureID);                    
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

    // Tangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(ProjectModals::Vertex), (void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::Tangent));

    // BiTangent
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(ProjectModals::Vertex),(void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::Bitangent));

    // ids
    glEnableVertexAttribArray(6);
    glVertexAttribIPointer(6, 4, GL_INT, sizeof(ProjectModals::Vertex), (void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::m_BoneIDs));

    // weights
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(ProjectModals::Vertex),(void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::m_Weights));   

    glBindVertexArray(0);
}

std::vector<ProjectModals::Vertex> Mesh::GetWorldVertices()
{
    return vertices;
}

std::vector<unsigned int> Mesh::GetIndices()
{
    return indices;
}
