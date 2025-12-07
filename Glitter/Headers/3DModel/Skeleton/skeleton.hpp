#pragma once
#include <vector>
#include <map>
#include <string>
#include <3DModel/Skeleton/AnimData.hpp>
#include <Helpers/Shader.hpp>
#include <Renderable/renderable.hpp>
#include <3DModel/Animation/Animator.hpp>
#include <Serializable.hpp>


namespace Skeleton {
    class Skeleton: public Serializable {
    
    public:
        Skeleton(): Serializable()
        {
        }

        std::map<std::string, BoneInfo> m_BoneInfoMap;
        int m_BoneCounter = 0;
        std::vector<glm::vec3> bonePositions;
        std::vector<glm::vec3> boneColors;
        unsigned int bonesVAO;
        unsigned int bonesVBO;
        unsigned int bonesColorVBO;
        Shader* bonesShader;
        std::string filename;

        void static ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
        {
            assert(src);
    
            dest.name = src->mName.data;
            dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
            dest.childrenCount = src->mNumChildren;
    
            for (int i = 0; i < src->mNumChildren; i++)
            {
                auto newData = std::make_shared<AssimpNodeData>();
                ReadHierarchyData(*newData, src->mChildren[i]);
                dest.children.push_back(newData);
            }
        }

        glm::mat4 getAccumulatedLocalTransform(const AssimpNodeData& node, const std::string& boneName, glm::mat4 accumulatedTransform = glm::mat4(1.0f), bool shouldAccumulate = false)
        {
            bool currentIsHelperNode = node.name.find(boneName) != std::string::npos; // ✅ Only accumulate if the node is related to `boneName`
            
            if ((currentIsHelperNode && node.name != boneName) || (shouldAccumulate))
            {
                shouldAccumulate = true; // Start accumulating transforms once we hit a related helper node
                accumulatedTransform *= node.transformation;
            }
            else
            {
                accumulatedTransform = glm::mat4(1.0f); // so we don't accumlate and  influence other unrelated bones.
                shouldAccumulate = false;
            }
        
        
            if (node.name == boneName)
            {
              if(shouldAccumulate)
              {
                  shouldAccumulate = false;
                  return accumulatedTransform; // ✅ Return final transformation when reaching the actual bone
              }
              return node.transformation;
            }
        
            for (const auto& child : node.children)
            {
                glm::mat4 result = getAccumulatedLocalTransform(*child, boneName, accumulatedTransform, shouldAccumulate);
                if (result != glm::mat4(1.0f)) // ✅ If valid transformation found, return it
                    return result;
            }
        
            return glm::mat4(1.0f); // ❌ If bone not found, return identity (safe fallback)
        }
        

        std::map<int, std::string> CreateBoneIndexMap(const std::map<std::string, BoneInfo>& boneInfoMap)
        {
            std::map<int, std::string> indexToName;
            for (const auto& [boneName, boneInfo] : boneInfoMap)
            {
                indexToName[boneInfo.id] = boneName;
            }
            return indexToName;
        }

        void BuildBoneHierarchy()
        {
            std::map<std::string, std::shared_ptr<AssimpNodeData>> nodeMap;
            std::map<int, std::string> indexToName = CreateBoneIndexMap(m_BoneInfoMap); // 🔥 Get index → name map
        
            // Step 1: Create nodes for all bones
            for (const auto& [boneName, boneInfo] : m_BoneInfoMap)
            {
                nodeMap[boneName] = std::make_shared<AssimpNodeData>(
                    glm::mat4(1.0f),  // Default transform (will update later)
                    boneName,
                    0,  // Children count (will update later)
                    std::vector<std::shared_ptr<AssimpNodeData>>()
                );
            }

            std::string rootBoneName = "";
        
            // Step 2: Attach children to their parents using parentIndex
            for (const auto& [boneName, boneInfo] : m_BoneInfoMap)
            {                
                int parentIndex = boneInfo.parentIndex;
                if (parentIndex != -1 && indexToName.find(parentIndex) != indexToName.end())  
                {
                    std::string parentName = indexToName[parentIndex];
                    nodeMap[parentName]->children.push_back(nodeMap[boneName]); // See this is attach empty default values for the children.
                    nodeMap[parentName]->childrenCount++;
                }
                else  
                {
                    // If there's no parent, it's the root bone
                    rootBoneName = boneName;
                }

                if(boneName == "mixamorig:LeftUpLeg")
                {
                    std::cout << "true";
                }

                //Look up RootNodedata and set local node transforms for skeletaltree nodes
                nodeMap[boneName]->transformation = getAccumulatedLocalTransform(m_RootNode, boneName);
            }

            //After all the bones have been parsed we move on to the rootNode i.e. represent the origin of the model.
            std::string rootNodeName = m_RootNode.name;
            nodeMap[rootNodeName] = std::make_shared<AssimpNodeData>(
                m_RootNode.transformation,
                rootNodeName,
                0,
                std::vector<std::shared_ptr<AssimpNodeData>>()
            );
            nodeMap[rootNodeName]->children.push_back(nodeMap[rootBoneName]);
            nodeMap[rootNodeName]->childrenCount++;

            skeletaltreeRoot = nodeMap[rootNodeName];
        }

        AssimpNodeData m_RootNode; 
        std::shared_ptr<AssimpNodeData> skeletaltreeRoot; 

        std::vector<Bone> m_Bones;//List Of Bones...

        void updateModelAndViewPosMatrix(Camera* camera, glm::mat4 &modelMatrix);
        void setupBoneBuffersOnGPU();

        glm::mat4 worldTransform(int boneIndex, glm::mat4 modelMatrix);
        bool isClose(glm::vec3 parentEndpoint, glm::vec3 childPosition, float tolerance);

        void draw(Camera* camera, glm::mat4 &modelMatrix);
        void setup(std::string filename);

    protected:
        virtual const std::string typeName() const override {return "skeleton"; }
        virtual const std::string contentName() override {return filename; }

        virtual void saveContent(fs::path contentFileLocation, std::ostream& os) override;
        virtual void loadContent(fs::path contentFileLocation, std::istream& is) override;

    private:

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & m_BoneInfoMap;
            ar & m_BoneCounter;
            ar & bonePositions;
            ar & m_RootNode;
            ar & m_Bones;
            ar & skeletaltreeRoot;
        }

    };
}