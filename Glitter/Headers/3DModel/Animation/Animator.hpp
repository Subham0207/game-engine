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
		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
		}
		else{
			m_CurrentTime = 0;
		}
		bonePositions.clear();
		//Use NodeDataFrom Skeleton
		// std::cout << "Skeletal Start" << std::endl;
		auto globalInverseTransform = glm::inverse(node->transformation); // make sure the first node is the rootNode and not the firstBone. This is used to position the model in the world space.
		CalculateBoneTransform(node, globalInverseTransform, boneInfoMap, modelMatrix, bonePositions, bones, globalInverseTransform);// This identity matrix is the reason the model doesnot move around the world. It's root stays at the origin
		// std::cout << "Skeletal End" << std::endl;
		
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

	void CalculateBoneTransform(
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
			else
			{
				std::cout << "node not animated " << nodeName << std::endl;
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

			if (node->childrenCount > 0) {
				glm::mat4 childGlobalTransform = globalTransformation * node->children[0].transformation;
				currentBonePosition = glm::vec3(childGlobalTransform * glm::vec4(0, 0, 0, 1));
			}

			//This is the perfect place to draw a bone; we have parent and child transform
			calculateBoneVectorFromParentAndChild(
				modelMatrix,
				bonePositions,
				boneInfoMap,
				currentBonePosition,
				parentBonePosition
			);
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation, boneInfoMap, modelMatrix, bonePositions, bones, globalInverseTransform);
	}

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
	
private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	float m_CurrentTime;
	float m_DeltaTime;

	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & m_FinalBoneMatrices;
        ar & m_CurrentAnimation;
        ar & m_CurrentTime;
        ar & m_DeltaTime;
    }
};