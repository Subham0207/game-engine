#pragma once
#include <vector>
#include <map>
#include <string>
#include <3DModel/Skeleton/AnimData.hpp>
#include <Helpers/Shader.hpp>
#include <Renderable/renderable.hpp>
#include <3DModel/Animation/Animator.hpp>


namespace Skeleton {
    class Skeleton {
    
    public:
        Skeleton()
        {
        }

        std::map<std::string, BoneInfo> m_BoneInfoMap;
        int m_BoneCounter = 0;
        std::vector<glm::vec3> bonePositions;
        unsigned int bonesVAO;
        unsigned int bonesVBO;
        Shader* bonesShader;
        Animator* animator;

        void updateModelAndViewPosMatrix(Camera* camera, glm::mat4 &modelMatrix);
        void setupBoneBuffersOnGPU();

        glm::mat4 worldTransform(int boneIndex, glm::mat4 modelMatrix);
        bool isClose(glm::vec3 parentEndpoint, glm::vec3 childPosition, float tolerance);

        void draw(Camera* camera, glm::mat4 &modelMatrix);
        void setup(Animator* animator, glm::mat4 modelMatrix);

    private:
    };
}