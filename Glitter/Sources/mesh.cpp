#include "3DModel/mesh.hpp"
#include <EngineState.hpp>
#include <Camera/Camera.hpp>

#include "Helpers/Shared.hpp"
#include "Modals/3DModelType.hpp"

Mesh::Mesh(std::vector<ProjectModals::Vertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Materials::IMaterial> material)
{
    this->vertices = vertices;
    this->indices = indices;

    setupMesh();

    if (material)
    {
        materialAssetGuid = material->getAssetId();
        std::cout << "materialAssetGuid set to = " << materialAssetGuid << std::endl;
        mMaterial = material;
    }
}

void Mesh::DrawOnlyGeometry()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::setFinalBoneMatrix(int boneIndex, glm::mat4 transform) const
{
    mMaterial->GetShader()->setMat4("finalBonesMatrices[" + std::to_string(boneIndex) + "]", transform);
}

void Mesh::Draw(Camera* camera, Lights* lightSystem, ModelType modelType, glm::mat4 modelMatrix, const std::vector<glm::mat4>* finalBoneMatrix)
{
    auto resolvedMaterial = mMaterial != nullptr ? mMaterial: EngineState::state->defaultMaterialInstance;

    resolvedMaterial->Bind();
    auto shaderProgramId = resolvedMaterial->GetShader()->ID;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramId, "dirLightVP"), 1, GL_FALSE, glm::value_ptr(lightSystem->directionalLights[0].dirLightVP));
    camera->updateMVP(shaderProgramId);

    auto cameraPosition = camera->getPosition();
    resolvedMaterial->GetShader()->setMat4("model", modelMatrix);
    resolvedMaterial->GetShader()->setBool("uHasTangents", HasTangents);
    glUniform3f(glGetUniformLocation(shaderProgramId, "viewPos"), cameraPosition.r, cameraPosition.g, cameraPosition.b);

    if (finalBoneMatrix)
    {
        resolvedMaterial->GetShader()->setBool("isAnimated", true);
        for (int boneIndex = 0; boneIndex < finalBoneMatrix->size(); ++boneIndex)
        {
            setFinalBoneMatrix(boneIndex, finalBoneMatrix->at(boneIndex));
        }
    }
    else
    {
        resolvedMaterial->GetShader()->setBool("isAnimated", false);
    }

    if(modelType == ModelType::ACTUAL_MODEL)
        lightSystem->Render(shaderProgramId);


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

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Binds VAO -> VBO
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ProjectModals::Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // Binds VAO -> EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    // GL_FALSE -- is for normalization; And Helps in saving space say in color attribute where we can pass 4Bit values for color ( 255, 255, 255 )
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

    // Tangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(ProjectModals::Vertex), (void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::Tangent));

    // BiTangent
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(ProjectModals::Vertex),(void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::Bitangent));

    // ids
    glEnableVertexAttribArray(6);
    glVertexAttribIPointer(6, MAX_BONE_INFLUENCE, GL_INT, sizeof(ProjectModals::Vertex), (void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::m_BoneIDs));

    // weights
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(ProjectModals::Vertex),(void*)offsetof(ProjectModals::Vertex, ProjectModals::Vertex::m_Weights));

    glBindVertexArray(0);


    //Set has tangents
    for (auto& vertex : vertices)
    {
        if(vertex.Tangent == glm::vec3(0.0f) || vertex.Bitangent == glm::vec3(0.0f))
        {
            this->HasTangents = false;
            return;
        }
    }
    this->HasTangents = true;
}

void Mesh::setupMaterial()
{
    if(materialAssetGuid.empty())
    {
        std::cout << "Material Asset: " << materialAssetGuid << std::endl;
       return;
    }

    std::cout << "Loading material: " << materialAssetGuid << "STARTED" << std::endl;
    auto file = fs::path(getEngineRegistryFilesMap()[materialAssetGuid]).string();
    if (Shared::endsWith(file, ".material"))
    {
        mMaterial = Materials::Material::loadMaterial(materialAssetGuid);
    }
    if (Shared::endsWith(file, ".materialInstance"))
    {
        mMaterial = Materials::MaterialInstance::loadMaterialInstance(materialAssetGuid);
        std::cout << "Loading material: " << materialAssetGuid << "FINISHED" << std::endl;
    }
}

std::vector<ProjectModals::Vertex> Mesh::GetWorldVertices()
{
    return vertices;
}

std::vector<unsigned int> Mesh::GetIndices()
{
    return indices;
}
