#pragma once
#include <Modals/vertex.hpp>
#include <assimp/scene.h>
#include <3DModel/Skeleton/AnimData.hpp>
#include <string>

#define MAX_BONE_WEIGHTS 100

namespace Helpers{
    void ExtractBoneWeightForVertices(
        std::vector<ProjectModals::Vertex>& vertices,
        aiMesh* mesh,
        const aiScene* scene,
        std::map<std::string, BoneInfo>& m_BoneInfoMap,
        int& m_BoneCounter);

    void resolveBoneHierarchy(const aiNode *node, int parentIndex, std::map<std::string, BoneInfo> &boneInfoMap);

    void SetVertexBoneDataToDefault(ProjectModals::Vertex& vertex);

    void SetVertexBoneData(ProjectModals::Vertex& vertex, int boneID, float weight);

}
