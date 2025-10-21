#include <3DModel/Animation/Animator.hpp>
#include <EngineState.hpp>
#include <glm/glm.hpp>

void Animator::onPoseTransitionInProgress(
    const std::shared_ptr<AssimpNodeData> node,
    glm::mat4 parentTransform,
    std::map<std::string, BoneInfo> &boneInfoMap,
    glm::mat4 &globalInverseTransform
)
{
    currentTransitionTime += m_DeltaTime;
    float blendFactor = glm::clamp(currentTransitionTime / maxTransitionDuration, 0.0f, 1.0f);
    if (currentTransitionTime >= maxTransitionDuration) 
    {
        poseTransitionInProgress = false;
        currentTransitionTime = 0.0f;
    }
    else
    {
        CalculateBoneTransformDuringTransition
        (
            node,
            globalInverseTransform,
            boneInfoMap,
            globalInverseTransform,
            blendFactor
        );
    }
}

glm::mat4 Animator::blendTransforms(const glm::mat4& A, const glm::mat4& B, float t) {
    glm::vec3 scaleA, translationA, skew;
    glm::vec4 perspectiveA;
    glm::quat rotA;
    glm::decompose(A, scaleA, rotA, translationA, skew, perspectiveA);

    glm::vec3 scaleB, translationB;
    glm::quat rotB;
    glm::decompose(B, scaleB, rotB, translationB, skew, perspectiveA); // reuse variables

    glm::vec3 scale = glm::mix(scaleA, scaleB, t);
    glm::vec3 translation = glm::mix(translationA, translationB, t);
    glm::quat rotation = glm::slerp(rotA, rotB, t);

    glm::mat4 result = glm::translate(glm::mat4(1.0f), translation)
                     * glm::mat4_cast(rotation)
                     * glm::scale(glm::mat4(1.0f), scale);

    return result;
}

void Animator::CalculateBoneTransformDuringTransition(
    const std::shared_ptr<AssimpNodeData> node,
    glm::mat4 parentTransform,
    std::map<std::string, BoneInfo> &boneInfoMap,
    glm::mat4 &globalInverseTransform,
    float blendFactor
)
{
    std::string nodeName = node->name;

    glm::mat4 nodeTransform = node->transformation;

    glm::mat4 globalTransformation = parentTransform * nodeTransform;
    
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].id;
        
        glm::mat4 sourceTransform = transitionSourcePose->at(index);
        glm::mat4 destTransform = transitionDestinationPose->at(index);
        
        glm::mat4 blendedTransform = blendTransforms(sourceTransform, destTransform, blendFactor);

        globalTransformation = parentTransform * blendedTransform;

        boneInfoMap[nodeName].transform = globalTransformation;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        
        m_FinalBoneMatrices[index] = globalInverseTransform * globalTransformation * offset;

        glm::vec3 currentBonePosition = glm::vec3(globalTransformation * glm::vec4(0, 0, 0, 1));
        glm::vec3 parentBonePosition = glm::vec3(parentTransform * glm::vec4(0, 0, 0, 1));
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransformDuringTransition(node->children[i], globalTransformation, boneInfoMap, globalInverseTransform, blendFactor);
}

void Animator::CalculateBoneTransform(
    const std::shared_ptr<AssimpNodeData> node,
    glm::mat4 parentTransform,
    std::map<std::string, BoneInfo> &boneInfoMap,
    glm::mat4 &modelMatrix,
    std::vector<glm::vec3> &bonePositions,
    std::vector<Bone> &bones,
    glm::mat4 &globalInverseTransform,
    float currentTime
    )
{
    std::string nodeName = node->name;
    
    glm::mat4 nodeTransform = node->transformation;
    
    if(m_CurrentAnimation)
    {
        auto animBone = m_CurrentAnimation->FindBone(nodeName);
        if(animBone)
        {
            animBone->Update(currentTime);
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
        m_FinalBoneMatricesLocal[index] = nodeTransform;

        glm::vec3 currentBonePosition = glm::vec3(globalTransformation * glm::vec4(0, 0, 0, 1));
        glm::vec3 parentBonePosition = glm::vec3(parentTransform * glm::vec4(0, 0, 0, 1));

        //This is the perfect place to draw a bone; we have parent and child transform
        if(!EngineState::state->isPlay && getUIState().showDebugBone)
        calculateBoneVectorFromParentAndChild(
            modelMatrix,
            bonePositions,
            boneInfoMap,
            currentBonePosition,
            parentBonePosition
        );
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(node->children[i], globalTransformation, boneInfoMap, modelMatrix, bonePositions, bones, globalInverseTransform, currentTime);
}
void Animator::CalculateBoneTransformBlended(
    const std::shared_ptr<AssimpNodeData> node,
    glm::mat4 parentTransform,
    std::map<std::string, BoneInfo> &boneInfoMap,
    glm::mat4 &modelMatrix, std::vector<glm::vec3> &bonePositions,
    std::vector<Bone> &bones, glm::mat4 &globalInverseTransform)
{
    std::string nodeName = node->name;
    
    glm::mat4 nodeTransform = node->transformation;

    if(blendSelection->bottomLeft || blendSelection->bottomRight || blendSelection->topLeft || blendSelection->topRight)
    {

        //Since by Here we have the times selected we can do timewarping
        //Get correct timewarp curve from the timewarp map i.e. {animation1, animation2}: timewarpCurve 
        //get the timewarpped time: so bL -> bR, bL -> tL, bL -> tR. if we take bL as the source animation.
        auto timewarpcurveBL_BR = timewarpmap[{blendSelection->bottomLeft->blendPointIndex, blendSelection->bottomRight->blendPointIndex}];
        auto timewarpcurveBL_TL = timewarpmap[{blendSelection->bottomLeft->blendPointIndex, blendSelection->bottomRight->blendPointIndex}];
        auto timewarpcurveBL_TR = timewarpmap[{blendSelection->bottomLeft->blendPointIndex, blendSelection->bottomRight->blendPointIndex}];

        if(timewarpcurveBL_BR)
        currentTime2 = timewarpcurveBL_BR->evaluate(currentTime1);
        if(timewarpcurveBL_TL)
        currentTime3 = timewarpcurveBL_TL->evaluate(currentTime1);
        if(timewarpcurveBL_TR)
        currentTime4 = timewarpcurveBL_TR->evaluate(currentTime1);
        //Now we time warped all the animations based on 1 one's timing.

        Bone* boneBL = blendSelection->bottomLeft && blendSelection->bottomLeft->animation ? blendSelection->bottomLeft->animation->FindBone(nodeName) : nullptr;
        Bone* boneBR = blendSelection->bottomRight && blendSelection->bottomRight->animation ? blendSelection->bottomRight->animation->FindBone(nodeName): nullptr;
        Bone* boneTL = blendSelection->topLeft && blendSelection->topLeft->animation ? blendSelection->topLeft->animation->FindBone(nodeName): nullptr;
        Bone* boneTR = blendSelection->topRight && blendSelection->topRight->animation ? blendSelection->topRight->animation->FindBone(nodeName): nullptr;
        nodeTransform = calculateLocalInterpolatedtransformForBone(
            boneTL,
            boneTR,
            boneBL,
            boneBR,
            blendSelection->topLeftBlendFactor,
            blendSelection->topRightBlendFactor,
            blendSelection->bottomLeftBlendFactor,
            blendSelection->bottomRightBlendFactor,
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
        m_FinalBoneMatricesLocal[index] = nodeTransform;

        glm::vec3 currentBonePosition = glm::vec3(globalTransformation * glm::vec4(0, 0, 0, 1));
        glm::vec3 parentBonePosition = glm::vec3(parentTransform * glm::vec4(0, 0, 0, 1));

        //This is the perfect place to draw a bone; we have parent and child transform
        if(!EngineState::state->isPlay && getUIState().showDebugBone)
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

    float totalWeight = topLeftBlendFactor + topRightBlendFactor + bottomLeftBlendFactor + bottomRightBlendFactor;

    glm::quat rot = glm::normalize(
        glm::slerp(glm::slerp(rotBL, rotBR, bottomRightBlendFactor / (bottomLeftBlendFactor + bottomRightBlendFactor + 0.0001f)),
                   glm::slerp(rotTL, rotTR, topRightBlendFactor / (topLeftBlendFactor + topRightBlendFactor + 0.0001f)),
                   (topRightBlendFactor + topLeftBlendFactor) / totalWeight)
    );

    glm::mat4 rotate = glm::toMat4(rot);

    return translate * rotate * scale;
}

void Animator::setAnimationTime()
{
    if(blendSelection->bottomLeftBlendFactor == 1 && blendSelection->bottomLeft->animation)
    {
        currentTime1 += blendSelection->bottomLeft->animation->GetTicksPerSecond() * m_DeltaTime;
        currentTime1 = fmod(currentTime1, blendSelection->bottomLeft->animation->GetDuration());

        //This is because all the four blendpoints point to the same animation so might as well have same timing
        //Basically letting us blend b/w the same animation at same time.
        //Could be expensive though
        currentTime2 = currentTime1;
        currentTime3 = currentTime1;
        currentTime4 = currentTime1;
        return;
    }

    if (blendSelection->bottomRightBlendFactor == 1 && blendSelection->bottomRight->animation) {
        currentTime2 += blendSelection->bottomRight->animation->GetTicksPerSecond() * m_DeltaTime;
        currentTime2 = fmod(currentTime2, blendSelection->bottomRight->animation->GetDuration());

        currentTime1 = currentTime2;
        currentTime3 = currentTime2;
        currentTime4 = currentTime2;
        return;
    }

    if (blendSelection->topLeftBlendFactor == 1 && blendSelection->topLeft->animation) {
        currentTime3 += blendSelection->topLeft->animation->GetTicksPerSecond() * m_DeltaTime;
        currentTime3 = fmod(currentTime3, blendSelection->topLeft->animation->GetDuration());

        currentTime1 = currentTime3;
        currentTime2 = currentTime3;
        currentTime4 = currentTime3;
        return;
    }

    if (blendSelection->topRightBlendFactor == 1 && blendSelection->topRight->animation) {
        currentTime4 += blendSelection->topRight->animation->GetTicksPerSecond() * m_DeltaTime;
        currentTime4 = fmod(currentTime4, blendSelection->topRight->animation->GetDuration());

        currentTime1 = currentTime4;
        currentTime2 = currentTime4;
        currentTime3 = currentTime4;
        return;
    }

    if (blendSelection->bottomLeft && blendSelection->bottomLeft->animation) {
        currentTime1 += blendSelection->bottomLeft->animation->GetTicksPerSecond() * m_DeltaTime;
        currentTime1 = fmod(currentTime1, blendSelection->bottomLeft->animation->GetDuration());
    }

    if (blendSelection->bottomRight && blendSelection->bottomRight->animation) {
       currentTime2 += blendSelection->bottomRight->animation->GetTicksPerSecond() * m_DeltaTime;
       currentTime2 = fmod(currentTime2, blendSelection->bottomRight->animation->GetDuration());
    }

    if (blendSelection->topLeft && blendSelection->topLeft->animation) {
       currentTime3 += blendSelection->topLeft->animation->GetTicksPerSecond() * m_DeltaTime;
       currentTime3 = fmod(currentTime3, blendSelection->topLeft->animation->GetDuration());
    }

    if (blendSelection->topRight && blendSelection->topRight->animation) {
       currentTime4 += blendSelection->topRight->animation->GetTicksPerSecond() * m_DeltaTime;
       currentTime4 = fmod(currentTime4, blendSelection->topRight->animation->GetDuration());
    }
}