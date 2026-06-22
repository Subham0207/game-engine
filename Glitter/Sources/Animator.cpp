#include <3DModel/Animation/Animator.hpp>
#include <EngineState.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <limits>

void Animator::HandlePlayedTypeForTransition()
{
    //Animation to blendspace
    if (lastPlayedType == 1 && currentPlayedType == 0)
    {
        bsPoseTransitionInProgress = false;
        bsPoseTransitionStarted = true;

        animPoseTransitionInProgress = false;
        animPoseTransitionStarted = false;
    }

    //Blendspace to animation
    if (lastPlayedType == 0 && currentPlayedType == 1)
    {
        animPoseTransitionInProgress = false;
        animPoseTransitionStarted = true;

        bsPoseTransitionInProgress = false;
        bsPoseTransitionStarted = false;
    }
}

void Animator::onPoseTransitionInProgress(
    const std::shared_ptr<AssimpNodeData> node,
    glm::mat4 parentTransform,
    std::map<std::string, BoneInfo> &boneInfoMap,
    glm::mat4 &globalInverseTransform,
    bool& poseTransitionInProgress
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
    const glm::mat4 bindPoseTransform = node->transformation;
    glm::mat4 nodeTransform = bindPoseTransform;
    Bone* boneBL = nullptr;
    Bone* boneBR = nullptr;
    Bone* boneTL = nullptr;
    Bone* boneTR = nullptr;

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

        boneBL = blendSelection->bottomLeft && blendSelection->bottomLeft->animation ? blendSelection->bottomLeft->animation->FindBone(nodeName) : nullptr;
        boneBR = blendSelection->bottomRight && blendSelection->bottomRight->animation ? blendSelection->bottomRight->animation->FindBone(nodeName): nullptr;
        boneTL = blendSelection->topLeft && blendSelection->topLeft->animation ? blendSelection->topLeft->animation->FindBone(nodeName): nullptr;
        boneTR = blendSelection->topRight && blendSelection->topRight->animation ? blendSelection->topRight->animation->FindBone(nodeName): nullptr;
        nodeTransform = calculateLocalInterpolatedtransformForBone(
            boneTL,
            boneTR,
            boneBL,
            boneBR,
            blendSelection->topLeftBlendFactor,
            blendSelection->topRightBlendFactor,
            blendSelection->bottomLeftBlendFactor,
            blendSelection->bottomRightBlendFactor,
            bindPoseTransform
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
        m_BlendSourceBoneMatricesLocal[0][index] = calculateBoneLocalTransform(boneBL, currentTime1, bindPoseTransform);
        m_BlendSourceBoneMatricesLocal[1][index] = calculateBoneLocalTransform(boneBR, currentTime2, bindPoseTransform);
        m_BlendSourceBoneMatricesLocal[2][index] = calculateBoneLocalTransform(boneTL, currentTime3, bindPoseTransform);
        m_BlendSourceBoneMatricesLocal[3][index] = calculateBoneLocalTransform(boneTR, currentTime4, bindPoseTransform);

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

namespace
{
    struct RegionBoundaryCrossing
    {
        float boundaryTime = 0.0f;
        AnimationRuntimeEvent event;
    };
}

std::vector<AnimationRuntimeEvent> Animator::CollectRegionBoundaryEvents(
    const std::vector<AnimationRegion>& regions,
    float previousTime,
    float currentTime,
    float duration,
    bool wrapped)
{
    if (duration <= 0.0f)
        return {};

    std::vector<AnimationRuntimeEvent> events;
    const auto collectSegmentCrossings = [&events, &regions](float segmentStart, float segmentEnd)
    {
        if (segmentEnd < segmentStart)
            return;

        std::vector<RegionBoundaryCrossing> segmentCrossings;
        for (const auto& region : regions)
        {
            if (region.startTime > segmentStart && region.startTime <= segmentEnd)
                segmentCrossings.push_back({region.startTime, {AnimEventType::RegionEnter, region.name}});

            if (region.endTime > segmentStart && region.endTime <= segmentEnd)
                segmentCrossings.push_back({region.endTime, {AnimEventType::RegionExit, region.name}});
        }

        std::stable_sort(
            segmentCrossings.begin(),
            segmentCrossings.end(),
            [](const RegionBoundaryCrossing& lhs, const RegionBoundaryCrossing& rhs)
            {
                return lhs.boundaryTime < rhs.boundaryTime;
            });

        for (const auto& crossing : segmentCrossings)
            events.push_back(crossing.event);
    };

    if (wrapped)
    {
        collectSegmentCrossings(previousTime, duration);
        collectSegmentCrossings(0.0f, currentTime);
    }
    else
    {
        collectSegmentCrossings(previousTime, currentTime);
    }

    return events;
}

float Animator::ComputeAverageLocalPoseError(
    const std::vector<glm::mat4>& sourcePoseLocal,
    const std::vector<glm::mat4>& blendedPoseLocal)
{
    const size_t count = std::min(sourcePoseLocal.size(), blendedPoseLocal.size());
    if (count == 0)
        return 1.0f;

    float totalError = 0.0f;
    for (size_t i = 0; i < count; ++i)
    {
        glm::vec3 sourceScale(1.0f), blendedScale(1.0f), sourceTranslation(0.0f), blendedTranslation(0.0f), skew(0.0f);
        glm::vec4 perspective(0.0f);
        glm::quat sourceRotation = glm::identity<glm::quat>();
        glm::quat blendedRotation = glm::identity<glm::quat>();
        glm::decompose(sourcePoseLocal[i], sourceScale, sourceRotation, sourceTranslation, skew, perspective);
        glm::decompose(blendedPoseLocal[i], blendedScale, blendedRotation, blendedTranslation, skew, perspective);

        if (glm::length(sourceRotation) < 1e-5f)
            sourceRotation = glm::identity<glm::quat>();
        if (glm::length(blendedRotation) < 1e-5f)
            blendedRotation = glm::identity<glm::quat>();

        sourceRotation = glm::normalize(sourceRotation);
        blendedRotation = glm::normalize(blendedRotation);

        const float translationError = glm::clamp(glm::length(sourceTranslation - blendedTranslation), 0.0f, 1.0f);
        const float scaleError = glm::clamp(glm::length(sourceScale - blendedScale), 0.0f, 1.0f);
        const float rotationAngle = glm::abs(glm::angle(sourceRotation * glm::inverse(blendedRotation)));
        const float rotationError = glm::clamp(rotationAngle / glm::pi<float>(), 0.0f, 1.0f);
        totalError += (translationError + scaleError + rotationError) / 3.0f;
    }

    return totalError / static_cast<float>(count);
}

std::optional<size_t> Animator::SelectBestPoseMatchIndex(
    const std::vector<std::vector<glm::mat4>>& candidatePosesLocal,
    const std::vector<glm::mat4>& blendedPoseLocal,
    float threshold)
{
    std::optional<size_t> bestIndex;
    float bestError = std::numeric_limits<float>::max();
    for (size_t i = 0; i < candidatePosesLocal.size(); ++i)
    {
        const float error = ComputeAverageLocalPoseError(candidatePosesLocal[i], blendedPoseLocal);
        if (error < bestError)
        {
            bestError = error;
            bestIndex = i;
        }
    }

    if (!bestIndex.has_value() || bestError > threshold)
        return std::nullopt;
    return bestIndex;
}

void Animator::handleAnimationRegionEvents(float previousTime, float currentTime, bool wrapped, Animation* animation)
{
    if (animation == nullptr || animation->regions.empty())
        return;

    const auto events = CollectRegionBoundaryEvents(
        animation->regions,
        previousTime,
        currentTime,
        animation->GetDuration(),
        wrapped);

    for (const auto& event : events)
        enqueueAnimationEvent(event);
}

void Animator::handleBlendspaceRegionEvents(float previousTime1, float previousTime2, float previousTime3, float previousTime4)
{
    if (blendSelection == nullptr)
        return;

    std::vector<BlendRegionCandidate> candidates;
    auto collectCandidates = [&](int sourceIndex, Animation* animation, float previousTime, float currentTime)
    {
        if (animation == nullptr || animation->regions.empty())
            return;

        const bool wrapped = animation->GetDuration() > 0.0f && currentTime < previousTime;
        const auto events = CollectRegionBoundaryEvents(
            animation->regions,
            previousTime,
            currentTime,
            animation->GetDuration(),
            wrapped);

        for (const auto& event : events)
            candidates.push_back({event, sourceIndex});
    };

    collectCandidates(0, blendSelection->bottomLeft ? blendSelection->bottomLeft->animation : nullptr, previousTime1, currentTime1);
    collectCandidates(1, blendSelection->bottomRight ? blendSelection->bottomRight->animation : nullptr, previousTime2, currentTime2);
    collectCandidates(2, blendSelection->topLeft ? blendSelection->topLeft->animation : nullptr, previousTime3, currentTime3);
    collectCandidates(3, blendSelection->topRight ? blendSelection->topRight->animation : nullptr, previousTime4, currentTime4);

    if (candidates.empty())
        return;

    std::vector<std::vector<glm::mat4>> candidatePoses;
    candidatePoses.reserve(candidates.size());
    for (const auto& candidate : candidates)
        candidatePoses.push_back(m_BlendSourceBoneMatricesLocal[candidate.sourceIndex]);

    const auto selectedIndex = SelectBestPoseMatchIndex(candidatePoses, m_FinalBoneMatricesLocal, 0.10f);
    if (!selectedIndex.has_value())
        return;

    enqueueAnimationEvent(candidates[*selectedIndex].event);
}

void Animator::enqueueAnimationEvent(const AnimationRuntimeEvent& event)
{
    if (animationEventQueue == nullptr)
        return;

    animationEventQueue->push(event.type, event.regionName);
}

glm::mat4 Animator::calculateBoneLocalTransform(Bone* bone, float time, const glm::mat4& bindPoseTransform) const
{
    if (bone == nullptr)
        return bindPoseTransform;

    const glm::vec3 position = bone->InterpolatePositionVec(time);
    const glm::vec3 scale = bone->InterpolateScalingVec(time);
    const glm::quat rotation = bone->InterpolateRotationInQuat(time);

    return glm::translate(glm::mat4(1.0f), position)
        * glm::mat4_cast(rotation)
        * glm::scale(glm::mat4(1.0f), scale);
}