#include <3DModel/Skeleton/skeleton.hpp>
#include <GLFW/glfw3.h>


void Skeleton::Skeleton::extractBonePositions(int boneIndex, glm::mat4 transform)
{
    auto it = m_BoneInfoMap.begin();
    std::advance(it, boneIndex);
    auto boneinfo = it->second;
    glm::vec3 bonePosition = glm::vec3(transform * boneinfo.offset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    bonePositions.push_back(bonePosition);
}

void Skeleton::Skeleton::setupBoneBuffersOnGPU()
{
    glGenVertexArrays(1, &bonesVAO);
    glGenBuffers(1, &bonesVBO);
    glBindVertexArray(bonesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bonesVBO);

    glBufferData(GL_ARRAY_BUFFER, bonePositions.size() * sizeof(glm::vec3), &bonePositions[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}

void Skeleton::Skeleton::draw(Camera* camera, glm::mat4 &modelMatrix)
{
    bonesShader->use();

    updateModelAndViewPosMatrix(camera, modelMatrix);
    bonePositions.clear();
    auto transforms = animator->GetFinalBoneMatrices();
    for (int i = 0; i < m_BoneInfoMap.size(); ++i)
    {
        extractBonePositions(i, transforms[i]);
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glBindVertexArray(bonesVAO);

    // Update vertex buffer with bone positions
    glBindBuffer(GL_ARRAY_BUFFER, bonesVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, bonePositions.size() * sizeof(glm::vec3), bonePositions.data());

    // Render as points
    glDrawArrays(GL_POINTS, 0, bonePositions.size());
    glBindVertexArray(0);

    glDisable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
}
void Skeleton::Skeleton::setup(Animator *animator)
{
    this->animator = animator;
    bonesShader = new Shader("E:/OpenGL/Glitter/Glitter/Shaders/boneShader.vert", "E:/OpenGL/Glitter/Glitter/Shaders/boneShader.frag");
    bonesShader->use();
    auto transforms = animator->GetFinalBoneMatrices();
    for (int i = 0; i < m_BoneInfoMap.size(); ++i)
    {
        extractBonePositions(i, transforms[i]);
    }
    setupBoneBuffersOnGPU();
}

void Skeleton::Skeleton::updateModelAndViewPosMatrix(Camera* camera, glm::mat4 &modelMatrix){
    glm::mat4 projection = camera->projectionMatrix();
    glm::mat4 view = camera->viewMatrix();
    bonesShader->setMat4("projection", projection);
    bonesShader->setMat4("view", view);
    bonesShader->setMat4("model", modelMatrix);
}