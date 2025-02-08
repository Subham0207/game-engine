#include <3DModel/Skeleton/skeleton.hpp>
#include <EngineState.hpp>
#include <Sprites/text.hpp>
#include <GLFW/glfw3.h>
#include <imgui.h>

glm::mat4 Skeleton::Skeleton::worldTransform(int boneIndex, glm::mat4 modelMatrix)
{
    auto it = m_BoneInfoMap.begin();
    std::advance(it, boneIndex);
    auto boneinfo = it->second;
    return it->second.transform * modelMatrix;
}

bool Skeleton::Skeleton::isClose(glm::vec3 parentEndpoint, glm::vec3 childPosition, float tolerance)
{
    return glm::all(glm::epsilonEqual(parentEndpoint, childPosition, tolerance));;
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

    calculateBoneStartAndEndPos(modelMatrix);

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(bonesVAO);

    // Update vertex buffer with bone positions
    glBindBuffer(GL_ARRAY_BUFFER, bonesVBO);
    glBufferData(GL_ARRAY_BUFFER, bonePositions.size() * sizeof(glm::vec3), bonePositions.data(), GL_DYNAMIC_DRAW);

    // Render as points
    glDrawArrays(GL_LINES, 0, bonePositions.size());
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}
void Skeleton::Skeleton::setup(Animator *animator, glm::mat4 modelMatrix)
{
    this->animator = animator;
    bonesShader = new Shader("E:/OpenGL/Glitter/Glitter/Shaders/boneShader.vert", "E:/OpenGL/Glitter/Glitter/Shaders/boneShader.frag");
    bonesShader->use();
    auto transforms = animator->GetFinalBoneMatrices();

    calculateBoneStartAndEndPos(modelMatrix);

    setupBoneBuffersOnGPU();
}

void Skeleton::Skeleton::calculateBoneStartAndEndPos(glm::mat4 &modelMatrix)
{
    bonePositions.clear();
    auto transform = animator->GetFinalBoneMatrices();
    for (int i = 0; i < m_BoneInfoMap.size(); ++i)
    {
        auto it = m_BoneInfoMap.begin();
        std::advance(it, i);

        int parentIndex = it->second.parentIndex;

        if (parentIndex != -1) {

            auto it = m_BoneInfoMap.begin();
            std::advance(it, parentIndex);
            auto boneinfo = it->second;
            glm::mat4 parentTransform = it->second.transform * modelMatrix;

            // Parent bone direction and length
            glm::vec3 parentDirection = glm::normalize(it->second.offset[3]); // Extract from the translation column
            float parentLength = parentDirection.length();

            // Calculate endpoint of parent bone
            glm::vec3 parentEndpoint = glm::vec3(parentTransform[3]) * glm::vec3(0, 0, parentLength);

            // Transform child's position to parent's space
            glm::mat4 childTransform = worldTransform(i, modelMatrix);
            glm::vec3 childPosition = childTransform[3]; // Extract translation

            // Check if positions match (within a tolerance)
            float EPSILON = 1e-5f;
            if(isClose(parentEndpoint, childPosition, EPSILON)) {
                // The child bone is likely using "Keep Offset"
            }
            else{
                //Actual bone lines
                if(i < getActiveLevel().textSprites.size())
                {
                    getActiveLevel().textSprites.at(i)->updatePosition(childPosition);
                }
                else
                {
                    auto textSprite = new Sprites::Text(it->first, childPosition);
                    getActiveLevel().textSprites.push_back(textSprite);
                }
                bonePositions.push_back(childPosition);
                bonePositions.push_back(parentTransform[3]);
            }
        }
    }
}
void Skeleton::Skeleton::updateModelAndViewPosMatrix(Camera *camera, glm::mat4 &modelMatrix)
{
    glm::mat4 projection = camera->projectionMatrix();
    glm::mat4 view = camera->viewMatrix();
    bonesShader->setMat4("projection", projection);
    bonesShader->setMat4("view", view);
    bonesShader->setMat4("model", modelMatrix);
}