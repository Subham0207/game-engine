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
            boneTL,
            boneTR,
            boneBL,
            boneBR,
            blendSelection.topLeftBlendFactor,
            blendSelection.topRightBlendFactor,
            blendSelection.bottomLeftBlendFactor,
            blendSelection.bottomRightBlendFactor,
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
    Bone *boneTL, Bone *boneTR, Bone *boneBL, Bone *boneBR,
    float topLeftBlendFactor, float topRightBlendFactor, float bottomLeftBlendFactor, float bottomRightBlendFactor, glm::mat4 bindPoseTransform)
{

    auto m_CurrentTimeBL = currentTime1;
    auto m_CurrentTimeBR = currentTime2;
    auto m_CurrentTimeTL = currentTime3;
    auto m_CurrentTimeTR = currentTime4;

    glm::vec3 posBL = boneBL ? boneBL->InterpolatePositionVec(m_CurrentTimeBL):  glm::vec3(bindPoseTransform[3]);
    glm::vec3 posBR = boneBR ? boneBR->InterpolatePositionVec(m_CurrentTimeBR): posBL;
    glm::vec3 posTL = boneTL ? boneTL->InterpolatePositionVec(m_CurrentTimeTL): posBL;
    glm::vec3 posTR = boneTR ? boneTR->InterpolatePositionVec(m_CurrentTimeTR): posBL;

    // Bilinear interpolation for position
    glm::vec3 interpolatedPos = posTL * topLeftBlendFactor + posTR * topRightBlendFactor + posBL * bottomLeftBlendFactor + posBR * bottomRightBlendFactor;
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), interpolatedPos);

    glm::vec3 scaleBL = boneBL ? boneBL->InterpolateScalingVec(m_CurrentTimeBL): glm::vec3(glm::length(bindPoseTransform[0]), glm::length(bindPoseTransform[1]), glm::length(bindPoseTransform[2]));
    glm::vec3 scaleBR = boneBR ? boneBR->InterpolateScalingVec(m_CurrentTimeBR): scaleBL;
    glm::vec3 scaleTL = boneTL ? boneTL->InterpolateScalingVec(m_CurrentTimeTL): scaleBL;
    glm::vec3 scaleTR = boneTR ? boneTR->InterpolateScalingVec(m_CurrentTimeTR): scaleBL;

    // Bilinear interpolation for scale
    glm::vec3 interpolatedScale = scaleTL * topLeftBlendFactor + scaleTR * topRightBlendFactor + scaleBL * bottomLeftBlendFactor + scaleBR * bottomRightBlendFactor;
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), interpolatedScale);

    glm::quat rotBL = boneBL ? boneBL->InterpolateRotationInQuat(m_CurrentTimeBL): glm::quat_cast(bindPoseTransform);
    glm::quat rotBR = boneBR ? boneBR->InterpolateRotationInQuat(m_CurrentTimeBR): rotBL;
    glm::quat rotTL = boneTL ? boneTL->InterpolateRotationInQuat(m_CurrentTimeTL): rotBL;
    glm::quat rotTR = boneTR ? boneTR->InterpolateRotationInQuat(m_CurrentTimeTR): rotBL;

    // Bilinear interpolation for rotation (slerp each axis)
    glm::quat rotX1 = glm::slerp(rotBL, rotBR, topRightBlendFactor + bottomRightBlendFactor);  // Interpolates Bottom Row
    glm::quat rotX2 = glm::slerp(rotTL, rotTR, topRightBlendFactor + topLeftBlendFactor);      // Interpolates Top Row
    glm::quat interpolatedRot = glm::slerp(rotX1, rotX2, topLeftBlendFactor + topRightBlendFactor);
    glm::mat4 rotate = glm::toMat4(interpolatedRot);

    return translate * rotate * scale;
}

void Animator::setAnimationTime()
{
    if(blendSelection.bottomLeftBlendFactor == 1)
    {
        currentTime1 += blendSelection.bottomLeft->GetTicksPerSecond() * m_DeltaTime;
        currentTime1 = fmod(currentTime1, blendSelection.bottomLeft->GetDuration());

        currentTime2 = currentTime1;
        currentTime3 = currentTime1;
        currentTime4 = currentTime1;
        return;
    }

    if (blendSelection.bottomRightBlendFactor == 1) {
        currentTime2 += blendSelection.bottomRight->GetTicksPerSecond() * m_DeltaTime;
        currentTime2 = fmod(currentTime2, blendSelection.bottomRight->GetDuration());

        currentTime1 = currentTime2;
        currentTime3 = currentTime2;
        currentTime4 = currentTime2;
        return;
    }

    if (blendSelection.topLeftBlendFactor == 1) {
        currentTime3 += blendSelection.topLeft->GetTicksPerSecond() * m_DeltaTime;
        currentTime3 = fmod(currentTime3, blendSelection.topLeft->GetDuration());

        currentTime1 = currentTime3;
        currentTime2 = currentTime3;
        currentTime4 = currentTime3;
        return;
    }

    if (blendSelection.topRightBlendFactor == 1) {
        currentTime4 += blendSelection.topRight->GetTicksPerSecond() * m_DeltaTime;
        currentTime4 = fmod(currentTime4, blendSelection.topRight->GetDuration());

        currentTime1 = currentTime4;
        currentTime2 = currentTime4;
        currentTime3 = currentTime4;
        return;
    }

    if (blendSelection.bottomLeftBlendFactor == 1) {
        currentTime1 += blendSelection.bottomLeft->GetTicksPerSecond() * m_DeltaTime;
        currentTime1 = fmod(currentTime1, blendSelection.bottomLeft->GetDuration());
    }

    if (blendSelection.bottomRight) {
       currentTime2 += blendSelection.bottomRight->GetTicksPerSecond() * m_DeltaTime;
       currentTime2 = fmod(currentTime2, blendSelection.bottomRight->GetDuration());
    }

    if (blendSelection.topLeft) {
       currentTime3 += blendSelection.topLeft->GetTicksPerSecond() * m_DeltaTime;
       currentTime3 = fmod(currentTime3, blendSelection.topLeft->GetDuration());
    }

    if (blendSelection.topRight) {
       currentTime4 += blendSelection.topRight->GetTicksPerSecond() * m_DeltaTime;
       currentTime4 = fmod(currentTime4, blendSelection.topRight->GetDuration());
    }

}