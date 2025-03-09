#include <3DModel/Animation/Animator.hpp>
#include <EngineState.hpp>
#include <glm/glm.hpp>

void Animator::CalculateBoneTransform(
    const AssimpNodeData* node,
    glm::mat4 parentTransform,
    std::map<std::string, BoneInfo> &boneInfoMap,
    glm::mat4 &modelMatrix,
    std::vector<glm::vec3> &bonePositions,
    std::vector<Bone> &bones,
    glm::mat4 &globalInverseTransform
    )
{
    std::string nodeName = node->name;
    
    glm::mat4 nodeTransform = node->transformation;
    
    if(m_CurrentAnimation)
    {
        auto animBone = m_CurrentAnimation->FindBone(nodeName);
        if(animBone)
        {
            animBone->Update(m_CurrentTime);
            nodeTransform = animBone->GetLocalTransform();
        }
    }
    
    glm::mat4 globalTransformation = parentTransform * nodeTransform;
    
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].id;
        boneInfoMap[nodeName].transform = globalTransformation;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalInverseTransform * globalTransformation * offset;

        glm::vec3 currentBonePosition = glm::vec3(globalTransformation * glm::vec4(0, 0, 0, 1));
        glm::vec3 parentBonePosition = glm::vec3(parentTransform * glm::vec4(0, 0, 0, 1));

        //This is the perfect place to draw a bone; we have parent and child transform
        if(!State::state->isPlay && getUIState().showDebugBone)
        calculateBoneVectorFromParentAndChild(
            modelMatrix,
            bonePositions,
            boneInfoMap,
            currentBonePosition,
            parentBonePosition
        );
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(node->children[i], globalTransformation, boneInfoMap, modelMatrix, bonePositions, bones, globalInverseTransform);
}
void Animator::CalculateBoneTransformBlended(
    const AssimpNodeData *node,
    glm::mat4 parentTransform,
    std::map<std::string, BoneInfo> &boneInfoMap,
    glm::mat4 &modelMatrix, std::vector<glm::vec3> &bonePositions,
    std::vector<Bone> &bones, glm::mat4 &globalInverseTransform,
    Animation *animation1,
    Animation *animation2,
    float blendFactor)
{
    std::string nodeName = node->name;
    
    glm::mat4 nodeTransform = node->transformation;
    
    if(animation1 && animation2)
    {
        auto animBone1 = animation1->FindBone(nodeName);
        auto animBone2 = animation2->FindBone(nodeName);
        if(animBone1 && animBone2)
        {
            auto localInterpolatedTransform = calculateLocalInterpolatedtransformForBones(animBone1, animBone2, blendFactor);
            nodeTransform = localInterpolatedTransform;
        }
    }
    
    glm::mat4 globalTransformation = parentTransform * nodeTransform;
    
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].id;
        boneInfoMap[nodeName].transform = globalTransformation;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalInverseTransform * globalTransformation * offset;

        glm::vec3 currentBonePosition = glm::vec3(globalTransformation * glm::vec4(0, 0, 0, 1));
        glm::vec3 parentBonePosition = glm::vec3(parentTransform * glm::vec4(0, 0, 0, 1));

        //This is the perfect place to draw a bone; we have parent and child transform
        if(!State::state->isPlay && getUIState().showDebugBone)
        calculateBoneVectorFromParentAndChild(
            modelMatrix,
            bonePositions,
            boneInfoMap,
            currentBonePosition,
            parentBonePosition
        );
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransformBlended(
            node->children[i],
            globalTransformation,
            boneInfoMap, modelMatrix,
            bonePositions,
            bones,
            globalInverseTransform,
            animation1,
            animation2,
            blendFactor);
}
glm::mat4 Animator::calculateLocalInterpolatedtransformForBones(Bone *bone1, Bone *bone2, float blendFactor)
{
    auto pos1 = bone1->InterpolatePositionVec(currentTime1);
    auto pos2 = bone2->InterpolatePositionVec(currentTime2);

    //lerp
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::mix(pos1, pos2, blendFactor));

    auto scale1 = bone1->InterpolateScalingVec(currentTime1);
    auto scale2 = bone2->InterpolateScalingVec(currentTime2);

    //lerp
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::mix(scale1, scale2, blendFactor));

    auto rotation1 = bone1->InterpolateRotationInQuat(currentTime1);
    auto rotation2 = bone2->InterpolateRotationInQuat(currentTime2);

    //slerp
    glm::quat rotation = glm::slerp(rotation1, rotation2, blendFactor);

    return translate * glm::toMat4(rotation) * scale;
}