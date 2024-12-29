#include <3DModel/Skeleton/skeleton.hpp>
#include <GLFW/glfw3.h>
#include <imgui.h>


void Skeleton::Skeleton::extractBonePositions(int boneIndex, glm::mat4 transform)
{
    auto it = m_BoneInfoMap.begin();
    std::advance(it, boneIndex);
    auto boneinfo = it->second;
    glm::vec3 bonePosition = it->second.offset[3];
        
    bonePositions.push_back(bonePosition);
}

void Skeleton::Skeleton::setupBoneBuffersOnGPU()
{
    glGenVertexArrays(1, &bonesVAO);
    glGenBuffers(1, &bonesVBO);
    glBindVertexArray(bonesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bonesVBO);

    glBufferData(GL_ARRAY_BUFFER, bonePositions.size() * sizeof(glm::vec3), bonePositions.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}

void Skeleton::Skeleton::draw(Camera* camera, glm::mat4 &modelMatrix)
{
    bonesShader->use();

    updateModelAndViewPosMatrix(camera, modelMatrix);
    bonePositions.clear();
    auto transform = animator->GetFinalBoneMatrices();
    for (int i = 0; i < m_BoneInfoMap.size(); ++i)
    {
        auto it = m_BoneInfoMap.begin();
        std::advance(it, i);

        int parentIndex = it->second.parentIndex;

        if (parentIndex != -1) {
            glm::vec3 childPos = glm::vec3(it->second.transform[3]);

            parentIndex = it->second.parentIndex;
            auto parentBone = m_BoneInfoMap.begin();
            std::advance(parentBone, parentIndex);

            glm::vec3 parentPos = glm::vec3(parentBone->second.transform[3]);

            extractBonePositions(i,transform[i]);
            extractBonePositions(parentIndex,transform[parentIndex]);

            // bonePositions.push_back(childPos);//Start of line
            // bonePositions.push_back(parentPos);//End of line
        }
    }

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(bonesVAO);

    // Update vertex buffer with bone positions
    glBindBuffer(GL_ARRAY_BUFFER, bonesVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, bonePositions.size() * sizeof(glm::vec3), bonePositions.data());

    // Render as points
    glDrawArrays(GL_LINES, 0, bonePositions.size());
    glBindVertexArray(0);

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
        auto it = m_BoneInfoMap.begin();
        std::advance(it, i);

        int parentIndex = it->second.parentIndex;

        if (parentIndex != -1) {
            glm::vec3 childPos = glm::vec3(transforms[i][3]);
            glm::vec3 parentPos = glm::vec3(transforms[parentIndex][3]);
            bonePositions.push_back(childPos);
            bonePositions.push_back(parentPos);
        }
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