#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <array>
#include <optional>
#include <unordered_set>
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "Animation.hpp"
#include <Helpers/vertexBoneDataHelper.hpp>
#include <Controls/BlendSpace2D.hpp>
#include <Event/EventQueue.hpp>

class Animator
{
public:
	Animator()
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = NULL;
		m_FinalBoneMatrices.reserve(100);
		m_FinalBoneMatricesLocal.reserve(100);
		m_DeltaTime = 0.0f;
		m_startTime = 0.0f;
		m_ElapsedTime = 0.0f;

		currentTime1 = 0;
		currentTime2 = 0;
		currentTime3 = 0;
		currentTime4 = 0;

		blendSelection = NULL;

		for (int i = 0; i < 100; i++)
		{
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
			m_FinalBoneMatricesLocal.push_back(glm::mat4(1.0f));
			for (auto& sourcePose : m_BlendSourceBoneMatricesLocal)
				sourcePose.push_back(glm::mat4(1.0f));
		}
	}

	void HandlePlayedTypeForTransition();

	void UpdateAnimation(
		float dt,
		std::map<std::string, BoneInfo> &boneInfoMap,
		glm::mat4 &modelMatrix,
		std::vector<glm::vec3> &bonePositions,
		std::shared_ptr<AssimpNodeData> node,
		std::vector<Bone> &bones)
	{
		// if((!blendSelection.bottomLeft && !blendSelection.bottomRight && !blendSelection.topLeft && !blendSelection.topRight) || (!blendSelection.bottomLeft->animation && !blendSelection.bottomRight->animation
		// && !blendSelection.topLeft->animation && !blendSelection.topRight->animation))
		// return;

		m_DeltaTime = dt;
		bonePositions.clear();
		auto globalInverseTransform = glm::inverse(node->transformation); // make sure the first node is the rootNode and not the firstBone. This is used to position the model in the world space.

		if(blendSelection)
		{
			currentPlayedType = 0;
			const float previousTime1 = currentTime1;
			const float previousTime2 = currentTime2;
			const float previousTime3 = currentTime3;
			const float previousTime4 = currentTime4;

			HandlePlayedTypeForTransition();

			if(bsPoseTransitionStarted)
			{
				transitionSourcePose = new std::vector<glm::mat4>(m_FinalBoneMatricesLocal);
				currentTime1 = currentTime2 = currentTime3 = currentTime4 = 0.0f;
				CalculateBoneTransformBlended(
					node,
					globalInverseTransform,
					boneInfoMap,
					modelMatrix,
					bonePositions,
					bones,
					globalInverseTransform);
				transitionDestinationPose = new std::vector<glm::mat4>(m_FinalBoneMatricesLocal);
				bsPoseTransitionStarted = false;
				bsPoseTransitionInProgress = true;
			}

			if (bsPoseTransitionInProgress)
			{
				onPoseTransitionInProgress(
					node,
					globalInverseTransform,
					boneInfoMap,
					globalInverseTransform,
					bsPoseTransitionInProgress
				);
			}	
			else
			{
				setAnimationTime();
				CalculateBoneTransformBlended(
					node,
					globalInverseTransform,
					boneInfoMap,
					modelMatrix,
					bonePositions,
					bones,
					globalInverseTransform);
				handleBlendspaceRegionEvents(previousTime1, previousTime2, previousTime3, previousTime4);
			}
		}
		else if (m_CurrentAnimation)
		{
			currentPlayedType = 1;
			const float previousTime = m_CurrentTime;

			HandlePlayedTypeForTransition();

			if(animPoseTransitionStarted)
			{
				//Here m_FinalBoneMatricesLocal will be from last pose.
				transitionSourcePose = new std::vector<glm::mat4>(m_FinalBoneMatricesLocal);
				//Here we re-compute m_FinalBoneMatricesLocal for destination animation first frame.
				CalculateBoneTransform(
					node,
					globalInverseTransform,
					boneInfoMap,
					modelMatrix,
					bonePositions,
					bones,
					globalInverseTransform,
					0.0f					
				);
				transitionDestinationPose = new std::vector<glm::mat4>(m_FinalBoneMatricesLocal);
				animPoseTransitionStarted = false;
				animPoseTransitionInProgress = true;
			}

			
			if (animPoseTransitionInProgress)
			{
				onPoseTransitionInProgress(
					node,
					globalInverseTransform,
					boneInfoMap,
					globalInverseTransform,
					animPoseTransitionInProgress
				);
			}	
			else
			{
				if (isAnimationPlaying)
					m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * m_DeltaTime;
				if (m_LoopCurrentAnimation)
				{
					m_ElapsedTime = m_CurrentTime;
					m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
				}
				else
				{
					const float duration = m_CurrentAnimation->GetDuration();
					if (m_CurrentTime > duration)
						m_CurrentTime = duration;
					m_ElapsedTime = m_CurrentTime;
				}

				CalculateBoneTransform(
					node,
					globalInverseTransform,
					boneInfoMap,
					modelMatrix,
					bonePositions,
					bones,
					globalInverseTransform,
					m_CurrentTime);
				const bool wrapped = m_LoopCurrentAnimation && m_CurrentAnimation->GetDuration() > 0.0f && m_CurrentTime < previousTime;
				handleAnimationRegionEvents(previousTime, m_CurrentTime, wrapped, m_CurrentAnimation);
			}
		}
		else
		{
			CalculateBoneTransform(
				node,
				globalInverseTransform,
				boneInfoMap,
				modelMatrix,
				bonePositions,
				bones,
				globalInverseTransform,
				m_CurrentTime);
		}

		lastPlayedType = currentPlayedType;
	}

	void setAnimationEventQueue(AnimationEventQueue* queue)
	{
		animationEventQueue = queue;
	}

	static std::vector<AnimationRuntimeEvent> CollectRegionBoundaryEvents(
		const std::vector<AnimationRegion>& regions,
		float previousTime,
		float currentTime,
		float duration,
		bool wrapped
	);

	static float ComputeAverageLocalPoseError(
		const std::vector<glm::mat4>& sourcePoseLocal,
		const std::vector<glm::mat4>& blendedPoseLocal
	);

	static std::optional<size_t> SelectBestPoseMatchIndex(
		const std::vector<std::vector<glm::mat4>>& candidatePosesLocal,
		const std::vector<glm::mat4>& blendedPoseLocal,
		float threshold
	);

	void onPoseTransitionInProgress(
		const std::shared_ptr<AssimpNodeData> node,
		glm::mat4 parentTransform,
		std::map<std::string, BoneInfo> &boneInfoMap,
		glm::mat4 &globalInverseTransform,
		bool& poseTransitionInProgress
	);

	void initNoLoopAnimation()
	{
		m_startTime = m_DeltaTime;
		m_CurrentTime = 0.0f;
		m_ElapsedTime = 0.0f;
		animPoseTransitionStarted = true;
		animPoseTransitionInProgress = false;
	}

	void SetLoopCurrentAnimation(const bool shouldLoop)
	{
		m_LoopCurrentAnimation = shouldLoop;
	}

	void SetCurrentTimeSeconds(float seconds)
	{
		if (m_CurrentAnimation == nullptr)
			return;

		const float ticksPerSecond = glm::max(1.0f, m_CurrentAnimation->GetTicksPerSecond());
		const float duration = m_CurrentAnimation->GetDuration();
		const float targetTicks = glm::max(0.0f, seconds) * ticksPerSecond;
		if (m_LoopCurrentAnimation && duration > 0.0f)
			m_CurrentTime = fmod(targetTicks, duration);
		else
			m_CurrentTime = glm::clamp(targetTicks, 0.0f, duration);
		m_ElapsedTime = m_CurrentTime;
	}

	float GetCurrentTimeSeconds() const
	{
		if (m_CurrentAnimation == nullptr)
			return 0.0f;

		const float ticksPerSecond = glm::max(1.0f, m_CurrentAnimation->GetTicksPerSecond());
		return m_CurrentTime / ticksPerSecond;
	}

	void SetPlaying(const bool isPlaying)
	{
		isAnimationPlaying = isPlaying;
	}

	void PlayAnimation(Animation* pAnimation)
	{
		if(pAnimation)
		{
			m_CurrentAnimation = pAnimation;
			SetPlaying(true);
			blendSelection = NULL;
		}
	}

	void PlayAnimationBlended(BlendSelection* blendSelection)
	{
		this->blendSelection = blendSelection;
		if(blendSelection->bottomLeft && blendSelection->bottomRight && 
			blendSelection->topLeft && blendSelection->topRight)
		{
			isAnimationPlaying = true;
			m_CurrentAnimation = NULL;
		}
	}


	void CalculateBoneTransform(
		const std::shared_ptr<AssimpNodeData> node,
		glm::mat4 parentTransform,
		std::map<std::string, BoneInfo> &boneInfoMap,
		glm::mat4 &modelMatrix,
		std::vector<glm::vec3> &bonePositions,
		std::vector<Bone> &bones,
		glm::mat4 &globalInverseTransform,
		float currentTime
		);

	void CalculateBoneTransformDuringTransition(
		const std::shared_ptr<AssimpNodeData> node,
		glm::mat4 parentTransform,
		std::map<std::string, BoneInfo> &boneInfoMap,
		glm::mat4 &globalInverseTransform,
		float blendFactor
		);

	void CalculateBoneTransformBlended(
		const std::shared_ptr<AssimpNodeData> node,
		glm::mat4 parentTransform,
		std::map<std::string, BoneInfo> &boneInfoMap,
		glm::mat4 &modelMatrix,
		std::vector<glm::vec3> &bonePositions,
		std::vector<Bone> &bones,
		glm::mat4 &globalInverseTransform
		);

	Bone* FindBone(const std::string& name, std::vector<Bone> &bones)
	{
		auto iter = std::find_if(bones.begin(), bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == bones.end()) return nullptr;
		else return &(*iter);
	}

	const std::vector<glm::mat4>& GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}

	glm::mat4 worldTransform(int boneIndex, glm::mat4 &modelMatrix, std::map<std::string, BoneInfo> &m_BoneInfoMap)
	{
		auto it = m_BoneInfoMap.begin();
		std::advance(it, boneIndex);
		auto boneinfo = it->second;
		return it->second.transform * modelMatrix;
	}

	bool isClose(glm::vec3 parentEndpoint, glm::vec3 childPosition, float tolerance)
	{
		return glm::all(glm::epsilonEqual(parentEndpoint, childPosition, tolerance));;
	}

	void calculateBoneVectorFromParentAndChild(
		glm::mat4 &modelMatrix,
		std::vector<glm::vec3> &bonePositions,
		std::map<std::string, BoneInfo> &m_BoneInfoMap,
		glm::vec3 childPos,
		glm::vec3 parentPos)
	{
		float EPSILON = 1e-5f;
		if(isClose(parentPos, childPos, EPSILON)) {
			// The child bone is likely using "Keep Offset"
		}
		else{
			//Actual bone lines
			// if(i < getActiveLevel().textSprites.size())
			// {
			// 	getActiveLevel().textSprites.at(i)->updatePosition(childPosition);
			// }
			// else
			// {
			// 	auto textSprite = new Sprites::Text(it->first, childPosition);
			// 	getActiveLevel().textSprites.push_back(textSprite);
			// }
			// std::cout << "child pos";
			// std::cout << childPos[0] << " " ;
			// std::cout << childPos[1] << " ";
			// std::cout << childPos[2] <<  " " << std::endl;

			// std::cout << "parent  pos";
			// std::cout << parentPos[0] << " " ;
			// std::cout << parentPos[1] << " ";
			// std::cout << parentPos[2] <<  " " << std::endl;
			bonePositions.push_back(childPos);
			bonePositions.push_back(parentPos);
		}
	}

	bool isAnimationPlaying = false;
	AnimationEventQueue* animationEventQueue = nullptr;

	std::vector<glm::mat4> *transitionSourcePose;
	std::vector<glm::mat4> *transitionDestinationPose;

	Animation* m_CurrentAnimation;

	int lastPlayedType = 0; // 0 means blendspace, 1 means animation;
	int currentPlayedType = 0;

	bool animPoseTransitionStarted = false;
	bool animPoseTransitionInProgress = false;
	bool bsPoseTransitionStarted = false;
	bool bsPoseTransitionInProgress = false;
	float currentTransitionTime = 0;
	float maxTransitionDuration = 0.25;

	BlendSelection* blendSelection;
	float currentTime1;
	float currentTime2;
	float currentTime3;
	float currentTime4;

	float m_CurrentTime;
	float m_ElapsedTime;
	bool m_LoopCurrentAnimation = true;

	std::map<std::pair<int,int>, Animation3D::TimeWarpCurve*> timewarpmap; // pair{index of blendpoint, index of point blendpoint} like 1->3
	
private:
	struct BlendRegionCandidate
	{
		AnimationRuntimeEvent event;
		int sourceIndex = 0;
	};

	void handleAnimationRegionEvents(float previousTime, float currentTime, bool wrapped, Animation* animation);
	void handleBlendspaceRegionEvents(float previousTime1, float previousTime2, float previousTime3, float previousTime4);
	void enqueueAnimationEvent(const AnimationRuntimeEvent& event);
	glm::mat4 calculateBoneLocalTransform(Bone* bone, float time, const glm::mat4& bindPoseTransform) const;

	std::vector<glm::mat4> m_FinalBoneMatrices; // this is in world space
	std::vector<glm::mat4> m_FinalBoneMatricesLocal; // this is in bone's local space.
	std::array<std::vector<glm::mat4>, 4> m_BlendSourceBoneMatricesLocal;
	float m_DeltaTime;
	float maxDuration;
	float m_startTime;

	glm::mat4 calculateLocalInterpolatedtransformForBone(Bone *boneTL, Bone *boneTR, Bone *boneBL, Bone *boneBR,
		float topLeftBlendFactor, float topRightBlendFactor, float bottomLeftBlendFactor, float bottomRightBlendFactor, glm::mat4 bindPoseTransform);

	void setAnimationTime();

	glm::mat4 blendTransforms(const glm::mat4& A, const glm::mat4& B, float t);

	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & m_FinalBoneMatrices;
        ar & m_CurrentAnimation;
        ar & m_CurrentTime;
        ar & m_DeltaTime;
    }
};