#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "assimp/Importer.hpp"
#include <assimp/postprocess.h>
#include "3DModel/Skeleton/Bone.hpp"
#include <functional>
#include "3DModel/Skeleton/AnimData.hpp"
#include <serializeAClass.hpp>
#include <Serializable.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<std::shared_ptr<AssimpNodeData>> children;

	AssimpNodeData()=default;
	AssimpNodeData(glm::mat4 transform, std::string name, int childrenCount, std::vector<std::shared_ptr<AssimpNodeData>> children):
	transformation(transform), name(name), childrenCount(childrenCount), children(children){};

	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & transformation;
        ar & name;
        ar & childrenCount;
        ar & children;
    }
};

class Animation: public Serializable
{
public:
	Animation() = default;

	std::string GetClassId() const override { return "Animation"; }

	Animation(std::string& animationPath): Serializable()
	{
		generate_asset_guid();
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);
		auto animation = scene->mAnimations[0];
		m_Duration = animation->mDuration;
		m_TicksPerSecond = animation->mTicksPerSecond;
		aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		globalTransformation = globalTransformation.Inverse();
		ReadHierarchyData(m_RootNode, scene->mRootNode);
		readSkeletalAnimationData(animation);

		animationName = fs::path(animationPath).filename().string();
		hasMissingBones = true;
	}

	~Animation()
	{
	}

	Bone* FindBone(const std::string& name)
	{
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

	
	inline float GetTicksPerSecond() { return m_TicksPerSecond; }
	inline float GetDuration() { return m_Duration;}
	inline const AssimpNodeData& GetRootNode() { return m_RootNode; }

	std::string animationName;
	bool hasMissingBones;

	void readSkeletalAnimationData(
	const aiAnimation* animation)
	{
		int size = animation->mNumChannels;

		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			m_Bones.push_back(Bone(channel->mNodeName.data, channel));
		}
	}

protected:
    virtual const std::string typeName() const override {return "animation"; }
    virtual const std::string contentName() override {return animationName; }

    virtual void saveContent(fs::path contentFileLocation, std::ostream& os) override;
    virtual void loadContent(fs::path contentFileLocation, std::istream& is) override;


private:
	void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
	{
		assert(src);

		dest.name = src->mName.data;
		dest.transformation = AssimpHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		for (int i = 0; i < src->mNumChildren; i++)
		{
			auto newData = std::make_shared<AssimpNodeData>();
			ReadHierarchyData(*newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}
	float m_Duration;
	int m_TicksPerSecond;
	int totalAnimatedFrames;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode; // This is the skeletal tree

	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
		ar & m_Duration;
		ar & m_TicksPerSecond;
		ar & m_Bones, m_RootNode;
		ar & animationName;
		ar & hasMissingBones;
    }
};
