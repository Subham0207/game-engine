#include <Helpers/vertexBoneDataHelper.hpp>
#include <Helpers/AssimpGLMHelpers.hpp>

namespace Helpers
{
    void SetVertexBoneDataToDefault(ProjectModals::Vertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_WEIGHTS; i++)
        {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }

    void SetVertexBoneData(ProjectModals::Vertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_WEIGHTS; ++i)
        {
            if (vertex.m_BoneIDs[i] < 0)
            {
                vertex.m_Weights[i] = weight;
                vertex.m_BoneIDs[i] = boneID;
                break;
            }
        }
    }

    void ExtractBoneWeightForVertices(
        std::vector<ProjectModals::Vertex>& vertices,
        aiMesh* mesh,
        const aiScene* scene,
        std::map<std::string, BoneInfo>& m_BoneInfoMap,
        int& m_BoneCounter)
    {
        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = m_BoneCounter;
                newBoneInfo.parentIndex = -1;
                newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
                    mesh->mBones[boneIndex]->mOffsetMatrix);
                newBoneInfo.transform = glm::mat4(1.0f);
                m_BoneInfoMap[boneName] = newBoneInfo;
                boneID = m_BoneCounter;
                m_BoneCounter++;
            }
            else
            {
                boneID = m_BoneInfoMap[boneName].id;
            }
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId <= vertices.size());
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }

    void resolveBoneHierarchy(const aiNode *node, int parentIndex, std::map<std::string, BoneInfo> &boneInfoMap, std::vector<Bone> &m_Bones)
    {
        std::string nodeName = node->mName.C_Str();

        if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
            boneInfoMap[nodeName].parentIndex = parentIndex;
            parentIndex = boneInfoMap[nodeName].id;
            
            m_Bones.push_back(Bone(nodeName,
                boneInfoMap[nodeName].id));
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            resolveBoneHierarchy(node->mChildren[i], parentIndex, boneInfoMap, m_Bones);
        }

    }
}
