#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "Animation.hpp"
#include <Helpers/vertexBoneDataHelper.hpp>
#include <Controls/BlendSpace2D.hpp>

class Animator
{
public:
	Animator()
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = NULL;
		m_FinalBoneMatrices.reserve(100);
		m_DeltaTime = 0.0f;
		m_startTime = 0.0f;
		m_ElapsedTime = 0.0f;

		currentTime1 = 0;
		currentTime2 = 0;
		currentTime3 = 0;
		currentTime4 = 0;

		blendSelection = NULL;

		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}

	void UpdateAnimation(
		float dt,
		std::map<std::string, BoneInfo> &boneInfoMap,
		glm::mat4 &modelMatrix,
		std::vector<glm::vec3> &bonePositions,
		AssimpNodeData *node,
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
			setAnimationTime();
			CalculateBoneTransformBlended(
			node,
			globalInverseTransform,
			boneInfoMap,
			modelMatrix,
			bonePositions,
			bones,
			globalInverseTransform);
		}
		else if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * m_DeltaTime;
			m_ElapsedTime = m_CurrentTime;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());

			CalculateBoneTransform(
				node,
				globalInverseTransform,
				boneInfoMap,
				modelMatrix,
				bonePositions,
				bones,
				globalInverseTransform);
		}
	}

	void initNoLoopAnimation()
	{
		m_startTime = m_DeltaTime;
		m_CurrentTime = 0.0f;
	}

	void PlayAnimation(Animation* pAnimation)
	{
		if(pAnimation)
		{
			m_CurrentAnimation = pAnimation;
			// m_CurrentTime = 0.0f;
			isAnimationPlaying = true;
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
		const AssimpNodeData* node,
		glm::mat4 parentTransform,
		std::map<std::string, BoneInfo> &boneInfoMap,
		glm::mat4 &modelMatrix,
		std::vector<glm::vec3> &bonePositions,
		std::vector<Bone> &bones,
		glm::mat4 &globalInverseTransform
		);

	void CalculateBoneTransformBlended(
		const AssimpNodeData* node,
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

	std::vector<glm::mat4> GetFinalBoneMatrices()
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
	Animation* m_CurrentAnimation;

	BlendSelection* blendSelection;
	float currentTime1;
	float currentTime2;
	float currentTime3;
	float currentTime4;

	float m_CurrentTime;
	float m_ElapsedTime;

	std::map<std::pair<int,int>, Animation3D::TimeWarpCurve*> timewarpmap; // pair{index of blendpoint, index of point blendpoint} like 1->3
	
private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	float m_DeltaTime;
	float maxDuration;
	float m_startTime;

	glm::mat4 calculateLocalInterpolatedtransformForBone(Bone *boneTL, Bone *boneTR, Bone *boneBL, Bone *boneBR,
		float topLeftBlendFactor, float topRightBlendFactor, float bottomLeftBlendFactor, float bottomRightBlendFactor, glm::mat4 bindPoseTransform);

	void setAnimationTime();

	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & m_FinalBoneMatrices;
        ar & m_CurrentAnimation;
        ar & m_CurrentTime;
        ar & m_DeltaTime;
    }
};