#include <3DModel/capsulecolliderMesh.hpp>
#include <EngineState.hpp>
#include <Modals/vertex.hpp>
#include "Helpers/shader.hpp"
#include <glm/glm.hpp>
#include <cmath>
#include <EngineState.hpp>
#include <Modals/3DModelType.hpp>

CapsuleColliderModel::CapsuleColliderModel(float radius, float halfHeight){
    // getActiveLevel().addRenderable(this);
    model = createCapsuleModel(radius, halfHeight);
}

void CapsuleColliderModel::draw(float deltaTime, Camera* camera, Lights* lights, CubeMap* cubeMap)
{
    model->useAttachedShader();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    model->draw(deltaTime, camera, lights, cubeMap);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void CapsuleColliderModel::reGenerateCapsuleColliderMesh(float radius, float halfheight)
{
    model = createCapsuleModel(radius, halfheight); //TODO: add destructor to Model class.
}

std::vector<std::shared_ptr<Modals::Material>> CapsuleColliderModel::getMaterials()
{
    return model->getMaterials();
}

bool CapsuleColliderModel::ShouldRender() {
    return !EngineState::state->isPlay;
}

void CapsuleColliderModel::GenerateCapsuleMesh(std::vector<ProjectModals::Vertex>& vertices, std::vector<unsigned int>& indices, float radius, float halfHeight, int segments, int rings)
{
    float M_PI = 3.14f;
    vertices.clear();
    indices.clear();

    // Cylinder part
    for (int y = 0; y <= 1; ++y)
    {
    float ypos = (y == 0 ? -halfHeight : halfHeight);
    for (int i = 0; i <= segments; ++i)
    {
        float theta = (float)i / segments * 2.0f * M_PI;
        float x = cos(theta) * radius;
        float z = sin(theta) * radius;
        auto vertex = ProjectModals::Vertex();
        vertex.Position = glm::vec3(x, ypos, z);
        vertices.emplace_back(vertex);
    }
    }

    // Cylinder indices
    int base = 0;
    for (int i = 0; i < segments; ++i)
    {
    int i0 = base + i;
    int i1 = base + i + 1;
    int i2 = base + i + segments + 1;
    int i3 = base + i + segments + 2;

    indices.push_back(i0);
    indices.push_back(i2);
    indices.push_back(i1);

    indices.push_back(i1);
    indices.push_back(i2);
    indices.push_back(i3);
    }

    // Hemisphere helper function
    auto addHemisphere = [&](bool top)
    {
    float offsetY = top ? halfHeight : -halfHeight;
    float sign = top ? 1.0f : -1.0f;
    int startIndex = vertices.size();

    for (int y = 0; y <= rings; ++y)
    {
        float v = (float)y / rings;
        float phi = (v * M_PI / 2.0f); // half sphere
        for (int x = 0; x <= segments; ++x)
        {
            float u = (float)x / segments;
            float theta = u * 2.0f * M_PI;
            float sx = radius * cos(theta) * sin(phi);
            float sy = radius * cos(phi) * sign;
            float sz = radius * sin(theta) * sin(phi);
            auto vertex = ProjectModals::Vertex();
            vertex.Position = glm::vec3(sx, sy + offsetY, sz);
            vertices.emplace_back(vertex);
        }
    }

    int ringVerts = segments + 1;
    for (int y = 0; y < rings; ++y)
    {
        for (int x = 0; x < segments; ++x)
        {
            int i0 = startIndex + y * ringVerts + x;
            int i1 = i0 + 1;
            int i2 = i0 + ringVerts;
            int i3 = i2 + 1;

            if (top) {
                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i1);

                indices.push_back(i1);
                indices.push_back(i2);
                indices.push_back(i3);
            } else {
                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i2);

                indices.push_back(i1);
                indices.push_back(i3);
                indices.push_back(i2);
            }
        }
    }
    };

    // Add hemispheres
    addHemisphere(true);  // top
    addHemisphere(false); // bottom
}

Model* CapsuleColliderModel::createCapsuleModel(float radius, float halfHeight, int segments, int rings)
{
    std::vector<ProjectModals::Vertex> capsuleVertices;
    std::vector<unsigned int> capsuleIndices;
    GenerateCapsuleMesh(capsuleVertices, capsuleIndices, radius, halfHeight, 16, 8);

    auto model = new Model();
    model->shader =  new Shader("./Shaders/staticShader.vert","./Shaders/staticShader.frag");
    model->meshes.push_back(Mesh(capsuleVertices, capsuleIndices));
    return model;
}

ModelType CapsuleColliderModel::getModelType() {return ModelType::COLLIDER;}