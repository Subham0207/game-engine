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
    
    if(blendSelection.bottomLeft && blendSelection.bottomRight && 
        blendSelection.topLeft && blendSelection.topRight)
    {

        Bone* boneBL = blendSelection.bottomLeft->FindBone(nodeName);
        Bone* boneBR = blendSelection.bottomRight->FindBone(nodeName);
        Bone* boneTL = blendSelection.topLeft->FindBone(nodeName);
        Bone* boneTR = blendSelection.topRight->FindBone(nodeName);
        if (boneBL && boneBR && boneTL && boneTR) {
            nodeTransform = calculateLocalInterpolatedtransformForBone(
                boneBL, boneBR, boneTL, boneTR,
                blendSelection.xFactor, blendSelection.yFactor
            );
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
            globalInverseTransform);
}
glm::mat4 Animator::calculateLocalInterpolatedtransformForBone(
    Bone *boneBL, Bone *boneBR, Bone *boneTL, Bone *boneTR,
    float xFactor, float yFactor)
{
    glm::vec3 posBL = boneBL->InterpolatePositionVec(currentTime1);
    glm::vec3 posBR = boneBR->InterpolatePositionVec(currentTime2);
    glm::vec3 posTL = boneTL->InterpolatePositionVec(currentTime3);
    glm::vec3 posTR = boneTR->InterpolatePositionVec(currentTime4);

    // Bilinear interpolation for position
    glm::vec3 posX1 = glm::mix(posBL, posBR, xFactor);
    glm::vec3 posX2 = glm::mix(posTL, posTR, xFactor);
    glm::vec3 interpolatedPos = glm::mix(posX1, posX2, yFactor);
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), interpolatedPos);

    glm::vec3 scaleBL = boneBL->InterpolateScalingVec(currentTime1);
    glm::vec3 scaleBR = boneBR->InterpolateScalingVec(currentTime2);
    glm::vec3 scaleTL = boneTL->InterpolateScalingVec(currentTime3);
    glm::vec3 scaleTR = boneTR->InterpolateScalingVec(currentTime4);

    // Bilinear interpolation for scale
    glm::vec3 scaleX1 = glm::mix(scaleBL, scaleBR, xFactor);
    glm::vec3 scaleX2 = glm::mix(scaleTL, scaleTR, xFactor);
    glm::vec3 interpolatedScale = glm::mix(scaleX1, scaleX2, yFactor);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), interpolatedScale);

    glm::quat rotBL = boneBL->InterpolateRotationInQuat(currentTime1);
    glm::quat rotBR = boneBR->InterpolateRotationInQuat(currentTime2);
    glm::quat rotTL = boneTL->InterpolateRotationInQuat(currentTime3);
    glm::quat rotTR = boneTR->InterpolateRotationInQuat(currentTime4);

    // Bilinear interpolation for rotation (slerp each axis)
    glm::quat rotX1 = glm::slerp(rotBL, rotBR, xFactor);
    glm::quat rotX2 = glm::slerp(rotTL, rotTR, xFactor);
    glm::quat interpolatedRot = glm::slerp(rotX1, rotX2, yFactor);
    glm::mat4 rotate = glm::toMat4(interpolatedRot);

    return translate * rotate * scale;
}

void Animator::setAnimationTime()
{
    if (blendSelection.bottomLeft && blendSelection.bottomRight && 
        blendSelection.topLeft && blendSelection.topRight) {

        currentTime1 += blendSelection.bottomLeft->GetTicksPerSecond() * m_DeltaTime;
        currentTime1 = fmod(currentTime1, blendSelection.bottomLeft->GetDuration());

        currentTime2 += blendSelection.bottomRight->GetTicksPerSecond() * m_DeltaTime;
        currentTime2 = fmod(currentTime2, blendSelection.bottomRight->GetDuration());

        currentTime3 += blendSelection.topLeft->GetTicksPerSecond() * m_DeltaTime;
        currentTime3 = fmod(currentTime3, blendSelection.topLeft->GetDuration());

        currentTime4 += blendSelection.topRight->GetTicksPerSecond() * m_DeltaTime;
        currentTime4 = fmod(currentTime4, blendSelection.topRight->GetDuration());
    }
    else
    {
        currentTime1 = 0.0f;
        currentTime2 = 0.0f;
        currentTime3 = 0.0f;
        currentTime4 = 0.0f;
    }

}