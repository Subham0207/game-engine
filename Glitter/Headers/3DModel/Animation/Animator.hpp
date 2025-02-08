#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "Animation.hpp"

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

	void UpdateAnimation(float dt, std::map<std::string, BoneInfo> &boneInfoMap)
	{
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f), boneInfoMap);
		}
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

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform, std::map<std::string, BoneInfo> &boneInfoMap)
	{
		std::string nodeName = node->name;
		// glm::mat4 nodeTransform = node->transformation;
		
		// Skip actual transformation logic and use identity matrices
		glm::mat4 nodeTransform = glm::mat4(1.0f);

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			boneInfoMap[nodeName].transform = globalTransformation;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
			m_FinalBoneMatrices[index] = globalTransformation * offset;

			//This is the perfect place to draw a bone; we have parent and child transform
			
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation, boneInfoMap);
	}

	std::vector<glm::mat4> GetFinalBoneMatrices()
	{
	// 	for (int i = 0; i < 100; i++) {
	// 	glm::mat4 matrix = m_FinalBoneMatrices[i];
	// 	std::cout <<"Bone " << i << " Matrix:" << std::endl;
	// 	for (int row = 0; row < 4; row++) {
	// 		for (int col = 0; col < 4; col++) {
	// 			std::cout << matrix[row][col] << " ";
	// 		}
	// 		std::cout << std::endl;
	// 	}
	// }
		return m_FinalBoneMatrices;
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