#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "Animation.hpp"
#include <Helpers/vertexBoneDataHelper.hpp>

class Animator
{
public:
	Animator()
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = NULL;
		animation1 = NULL;
		animation2 = NULL;
		blendFactor = 0.0f;
		m_FinalBoneMatrices.reserve(100);

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
		m_DeltaTime = dt;
		if (animation1 && animation2)
		{
			currentTime1 += animation1->GetTicksPerSecond() * dt;
			currentTime1 = fmod(currentTime1, animation1->GetDuration());

			currentTime2 += animation2->GetTicksPerSecond() * dt;
			currentTime2 = fmod(currentTime2, animation2->GetDuration());
		}
		else{
			m_CurrentTime = 0;
			currentTime1 = 0;
			currentTime2 = 0;
		}
		bonePositions.clear();
		//Use NodeDataFrom Skeleton
		// std::cout << "Skeletal Start" << std::endl;
		auto globalInverseTransform = glm::inverse(node->transformation); // make sure the first node is the rootNode and not the firstBone. This is used to position the model in the world space.
		CalculateBoneTransformBlended(
		node,
		globalInverseTransform,
		boneInfoMap,
		modelMatrix,
		bonePositions,
		bones,
		globalInverseTransform,
		animation1,
		animation2,
		blendFactor);
	}

	void PlayAnimation(Animation* pAnimation)
	{
		if(pAnimation)
		{
			m_CurrentAnimation = pAnimation;
			m_CurrentTime = 0.0f;
			isAnimationPlaying = true;
		}
	}

	void PlayAnimationBlended(Animation* animation1, Animation* animation2, float blendFactor)
	{
		if(animation1 && animation2)
		{
			this->animation1 = animation1;
			this->animation2 = animation2;
			this->blendFactor = blendFactor;
			isAnimationPlaying = true;
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
		glm::mat4 &globalInverseTransform,
		Animation* animation1,
		Animation* animation2,
		float blendFactor
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

	//These three values need to be passed from animStateMachine 
	Animation* animation1;
	Animation* animation2;
	float currentTime1;
	float currentTime2;
	float blendFactor;
	
private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	float m_CurrentTime;
	float m_DeltaTime;

	glm::mat4 calculateLocalInterpolatedtransformForBones(Bone* bone1, Bone* bone2, float blendFactor);

	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & m_FinalBoneMatrices;
        ar & m_CurrentAnimation;
        ar & m_CurrentTime;
        ar & m_DeltaTime;
    }
};