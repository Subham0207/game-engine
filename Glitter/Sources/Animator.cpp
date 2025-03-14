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
    std::vector<Bone> &bones, glm::mat4 &globalInverseTransform)
{
    std::string nodeName = node->name;
    
    glm::mat4 nodeTransform = node->transformation;

    if(blendSelection.bottomLeft || blendSelection.bottomRight || blendSelection.topLeft || blendSelection.topRight)
    {
        Bone* boneBL = blendSelection.bottomLeft ? blendSelection.bottomLeft->FindBone(nodeName) : nullptr;
        Bone* boneBR = blendSelection.bottomRight ? blendSelection.bottomRight->FindBone(nodeName): nullptr;
        Bone* boneTL = blendSelection.topLeft ? blendSelection.topLeft->FindBone(nodeName): nullptr;
        Bone* boneTR = blendSelection.topRight ? blendSelection.topRight->FindBone(nodeName): nullptr;
        nodeTransform = calculateLocalInterpolatedtransformForBone(
            boneBL, boneBR, boneTL, boneTR,
            blendSelection.xFactor, blendSelection.yFactor,
            nodeTransform
        );
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
            globalInverseTransform);
}
glm::mat4 Animator::calculateLocalInterpolatedtransformForBone(
    Bone *boneBL, Bone *boneBR, Bone *boneTL, Bone *boneTR,
    float xFactor, float yFactor, glm::mat4 bindPoseTransform)
{
    float t_normalized = m_CurrentTime / maxDuration;
    auto warpTime = [&](float animationDuration) -> float {
        return t_normalized * animationDuration;
    };

    auto m_CurrentTimeBL = blendSelection.bottomLeft ? warpTime(blendSelection.bottomLeft->GetDuration()) : 0.0f;
    auto m_CurrentTimeBR = blendSelection.bottomRight ? warpTime(blendSelection.bottomRight->GetDuration()) : 0.0f;
    auto m_CurrentTimeTL = blendSelection.topLeft ? warpTime(blendSelection.topLeft->GetDuration()) : 0.0f;
    auto m_CurrentTimeTR = blendSelection.topRight ? warpTime(blendSelection.topRight->GetDuration()) : 0.0f;

    glm::vec3 posBL = boneBL ? boneBL->InterpolatePositionVec(m_CurrentTimeBL):  glm::vec3(bindPoseTransform[3]);
    glm::vec3 posBR = boneBR ? boneBR->InterpolatePositionVec(m_CurrentTimeBR): posBL;
    glm::vec3 posTL = boneTL ? boneTL->InterpolatePositionVec(m_CurrentTimeTL): posBL;
    glm::vec3 posTR = boneTR ? boneTR->InterpolatePositionVec(m_CurrentTimeTR): posBL;

    // Bilinear interpolation for position
    glm::vec3 posX1 = glm::mix(posBL, posBR, xFactor);
    glm::vec3 posX2 = glm::mix(posTL, posTR, xFactor);
    glm::vec3 interpolatedPos = glm::mix(posX1, posX2, yFactor);
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), interpolatedPos);

    glm::vec3 scaleBL = boneBL ? boneBL->InterpolateScalingVec(m_CurrentTimeBL): glm::vec3(glm::length(bindPoseTransform[0]), glm::length(bindPoseTransform[1]), glm::length(bindPoseTransform[2]));
    glm::vec3 scaleBR = boneBR ? boneBR->InterpolateScalingVec(m_CurrentTimeBR): scaleBL;
    glm::vec3 scaleTL = boneTL ? boneTL->InterpolateScalingVec(m_CurrentTimeTL): scaleBL;
    glm::vec3 scaleTR = boneTR ? boneTR->InterpolateScalingVec(m_CurrentTimeTR): scaleBL;

    // Bilinear interpolation for scale
    glm::vec3 scaleX1 = glm::mix(scaleBL, scaleBR, xFactor);
    glm::vec3 scaleX2 = glm::mix(scaleTL, scaleTR, xFactor);
    glm::vec3 interpolatedScale = glm::mix(scaleX1, scaleX2, yFactor);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), interpolatedScale);

    glm::quat rotBL = boneBL ? boneBL->InterpolateRotationInQuat(m_CurrentTimeBL): glm::quat_cast(bindPoseTransform);
    glm::quat rotBR = boneBR ? boneBR->InterpolateRotationInQuat(m_CurrentTimeBR): rotBL;
    glm::quat rotTL = boneTL ? boneTL->InterpolateRotationInQuat(m_CurrentTimeTL): rotBL;
    glm::quat rotTR = boneTR ? boneTR->InterpolateRotationInQuat(m_CurrentTimeTR): rotBL;

    // Bilinear interpolation for rotation (slerp each axis)
    glm::quat rotX1 = glm::slerp(rotBL, rotBR, xFactor);
    glm::quat rotX2 = glm::slerp(rotTL, rotTR, xFactor);
    glm::quat interpolatedRot = glm::slerp(rotX1, rotX2, yFactor);
    glm::mat4 rotate = glm::toMat4(interpolatedRot);

    return translate * rotate * scale;
}

void Animator::setAnimationTime()
{
    maxDuration = 0.0f;
    float ticksPerSecond = 30.0f;
    if (blendSelection.bottomLeft)  
    {
        maxDuration = std::max(maxDuration, blendSelection.bottomLeft->GetDuration());
        ticksPerSecond = blendSelection.bottomLeft->GetTicksPerSecond();
    }
    if (blendSelection.bottomRight) 
    {
        maxDuration = std::max(maxDuration, blendSelection.bottomRight->GetDuration());
        ticksPerSecond = blendSelection.bottomRight->GetDuration();
    }
    if (blendSelection.topLeft)
    {
        maxDuration = std::max(maxDuration, blendSelection.topLeft->GetDuration());
        ticksPerSecond = blendSelection.topLeft->GetDuration();
    }
    if (blendSelection.topRight)    
    {
        maxDuration = std::max(maxDuration, blendSelection.topRight->GetDuration());
        ticksPerSecond = blendSelection.topRight->GetDuration();
    }

    if (maxDuration <= 0.0f) return;

    m_CurrentTime += ticksPerSecond * m_DeltaTime;
    m_CurrentTime = fmod(m_CurrentTime, maxDuration);

    m_ElapsedTime += m_DeltaTime;
}