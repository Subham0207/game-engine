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

        void extractBonePositions(int boneIndex, glm::mat4 transform);
        void updateModelAndViewPosMatrix(Camera* camera, glm::mat4 &modelMatrix);
        void setupBoneBuffersOnGPU();

        void draw(Camera* camera, glm::mat4 &modelMatrix);
        void setup(Animator* animator);

    private:
    };
}