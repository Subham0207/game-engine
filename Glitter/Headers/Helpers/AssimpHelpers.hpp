#pragma once

#include"assimp/quaternion.h"
#include"assimp/vector3.h"
#include"assimp/matrix4x4.h"
#include"glm/glm.hpp"
#include"glm/gtc/quaternion.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <iostream>
#include <Jolt/Jolt.h>
#include <Jolt/Math/Mat44.h>
#include "glm/gtc/type_ptr.hpp"

class AssimpHelpers
{
public:

	static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
	{
		glm::mat4 to;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}

	static inline glm::mat4 ConvertMatrixToGLMFormat(const JPH::RMat44& joltMat) {
		return glm::make_mat4(reinterpret_cast<const float*>(&joltMat));
	}

	static inline glm::vec3 toGlM(const JPH::RVec3& joltVec3)
	{
		return glm::make_vec3(reinterpret_cast<const float*>(&joltVec3));
	}

	static inline glm::quat toGlM(const JPH::Quat& joltQuat)
	{
		return glm::make_quat(reinterpret_cast<const float*>(&joltQuat));
	}

	static inline glm::vec3 GetGLMVec(const aiVector3D& vec) 
	{ 
		return glm::vec3(vec.x, vec.y, vec.z); 
	}

	static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
	{
		return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
	}

	static const aiScene* createAssimpScene(const std::string& path, Assimp::Importer& import)
	{
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
			return nullptr;
		}

		return scene;
	}

	static bool isSkinned(const aiScene* scene)
	{
		bool hasSkeletalData = false;

		if(scene)
		{
			for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
				if (scene->mMeshes[i]->HasBones()) {
					hasSkeletalData = true;
					break;
				}
			}
		}
		return hasSkeletalData;
	}
};