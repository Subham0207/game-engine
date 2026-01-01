#include <Helpers/vertexBoneDataHelper.hpp>
#include <Helpers/AssimpHelpers.hpp>
#include <iostream>

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
                auto bone = mesh->mBones[boneIndex];
                if (bone->mOffsetMatrix != aiMatrix4x4()) {
                    newBoneInfo.offset = AssimpHelpers::ConvertMatrixToGLMFormat(bone->mOffsetMatrix);
                } 
                else 
                {
                    // If offset matrix is missing, attempt to compute it from the node hierarchy
                    aiNode* boneNode = FindNodeRecursive(scene->mRootNode, boneName);
                    if (boneNode) {
                        newBoneInfo.offset = GetBoneOffsetMatrix(boneNode);
                    } 
                    else {
                        std::cerr << "Warning: Bone '" << boneName << "' missing from mesh and scene graph!" << std::endl;
                        newBoneInfo.offset = glm::mat4(1.0f); // Default to identity
                    }
                }
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

            if (parentIndex == -1){
                std::cout << "WARNING: Parent is missing! " << nodeName << std::endl;
            }
            
            m_Bones.push_back(Bone(nodeName,
                boneInfoMap[nodeName].id));
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            resolveBoneHierarchy(node->mChildren[i], parentIndex, boneInfoMap, m_Bones);
        }

    }

    glm::mat4 GetBoneOffsetMatrix(const aiNode* boneNode) {
        if (!boneNode) return glm::mat4(1.0f);
    
        aiMatrix4x4 inverseTransform = boneNode->mTransformation;
        inverseTransform.Inverse();  // Get the inverse bind pose
    
        return AssimpHelpers::ConvertMatrixToGLMFormat(inverseTransform);
    }

    aiNode* FindNodeRecursive(aiNode* node, const std::string& nodeName)
    {
        if (!node) return nullptr;

        if (node->mName.C_Str() == nodeName) {
            return node;
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            aiNode* foundNode = FindNodeRecursive(node->mChildren[i], nodeName);
            if (foundNode) return foundNode;
        }

        return nullptr;
    }

    bool IsMatrixIdentity(const glm::mat4& mat, float epsilon) {
        glm::mat4 identity = glm::mat4(1.0f);
        
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (!glm::epsilonEqual(mat[i][j], identity[i][j], epsilon)) {
                    return false;
                }
            }
        }
        return true;
    }
}
