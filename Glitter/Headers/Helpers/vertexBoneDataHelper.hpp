#pragma once
#include <Modals/vertex.hpp>
#include <assimp/scene.h>
#include <3DModel/Skeleton/AnimData.hpp>
#include <string>
#include <3DModel/Skeleton/Bone.hpp>

#define MAX_BONE_WEIGHTS 100

namespace Helpers{
    void ExtractBoneWeightForVertices(
        std::vector<ProjectModals::Vertex>& vertices,
        aiMesh* mesh,
        const aiScene* scene,
        std::map<std::string, BoneInfo>& m_BoneInfoMap,
        int& m_BoneCounter);

    void resolveBoneHierarchy(const aiNode *node, int parentIndex, std::map<std::string, BoneInfo> &boneInfoMap, std::vector<Bone> &m_Bones);

    void SetVertexBoneDataToDefault(ProjectModals::Vertex& vertex);

    void SetVertexBoneData(ProjectModals::Vertex& vertex, int boneID, float weight);

    glm::mat4 GetBoneOffsetMatrix(const aiNode* boneNode);

    aiNode* FindNodeRecursive(aiNode* node, const std::string& nodeName);
}
