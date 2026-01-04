#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/access.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "3DModel/model.hpp"
#include "Camera/Camera.hpp"
#include <Character/Character.hpp>
#include <Renderable/renderable.hpp>
#include <Sprites/text.hpp>
#include <Serializable.hpp>
namespace fs = std::filesystem;
namespace bs = boost::property_tree;

class dtNavMesh;
class dtNavMeshQuery;
class NavMeshBuilder;
namespace AI{
    class AI;
}

struct LevelDetails
{
    std::string modelFilePath;
    Model* modelFile;

    LevelDetails(std::string _modelFilePath, Model* _modelFile)
    : modelFilePath(_modelFilePath), modelFile(_modelFile) {}
};

class Level: public Serializable{
    public:
        Level();

        bool static checkIfLevelFileExists(std::string  filename)
        {
                if (fs::exists(filename)) {
                    return true;
                }
                return false;
        }

        void loadMainLevelOfCurrentProject();

        void addRenderable(const shared_ptr<Renderable>& renderable){
            modelFilePaths.push_back(renderable->getName());
            modelTransformations.push_back(&renderable->getModelMatrix());
            renderables.push_back(renderable);
        }

        void addAI(AI::AI* ai);

        std::vector<std::string> modelFilePaths;
        std::vector<glm::mat4*> modelTransformations;
        std::vector<Sprites::Text*> textSprites;
        std::vector<shared_ptr<Renderable>> renderables = std::vector<shared_ptr<Renderable>>();
        std::vector<AI::AI*> AIs;
        std::string levelname = "level1";

        std::vector<Camera*> cameras;

        std::map<std::string, std::shared_ptr<Serializable>> instanceIdToSerializableMap;

        void tickAIs(float deltaTime);

        const std::string contentName() override { return levelname;}
        const std::string typeName() const override {return "lvl";}

        std::string GetGuid() {
            return getAssetId();
        }
        void BuildLevelNavMesh();
        bool FindPath(
                  const float* startPos,
                  const float* endPos,
                  std::vector<float>& outPath);
        dtNavMesh* navMesh = nullptr;
        dtNavMeshQuery* navQuery = nullptr;
        bool isNavMeshSetup = false;

        bool SampleRandomPoint(float* outPt);

        // render merged mesh
        void setupLevelVertices(std::vector<float> navVerts, std::vector<unsigned int> navTris);
        void renderLevelvertices(Camera *camera);
        //-------------

        //------Navigation-Mesh---------
        void renderDebugNavMesh(Camera *camera);
        //--------
    protected:
        
        void saveContent(fs::path contentFile, std::ostream& os) override;
        void loadContent(fs::path contentFile, std::istream& is) override;

    private:
   
        // Details for 1 mesh for by mergeing all models in the level
        std::vector<float> verts;
        std::vector<unsigned int>   tris;
        Mesh* lvlVerticesMesh = nullptr;
        Shader* lvlVerticesShader = nullptr;
        //-------------------------------

        //------Navigation-Mesh---------
        Mesh* debugNavMesh = nullptr;
        Shader* debugNavMeshShader = nullptr;
        NavMeshBuilder* builder = nullptr;
        //-------------
        
        static float frand()
        {
            return (float)std::rand() / (float)RAND_MAX;
        }

    static glm::vec3 readVec3(const bs::ptree& parent, const std::string& key) {
        glm::vec3 v;
        int i = 0;
        for (const auto& item : parent.get_child(key)) {
            v[i++] = std::stof(item.second.data());
        }
        return v;
    }

    static glm::quat readQuat(const bs::ptree& parent, const std::string& key) {
        glm::quat v;
        int i = 0;
        for (const auto& item : parent.get_child(key)) {
            v[i++] = std::stof(item.second.data());
        }
        return v;
    }

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & modelFilePaths;
            ar & modelTransformations;
            ar & levelname;
        }
};