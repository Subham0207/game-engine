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

struct LevelDetails
{
    std::string modelFilePath;
    Model* modelFile;

    LevelDetails(std::string _modelFilePath, Model* _modelFile)
    : modelFilePath(_modelFilePath), modelFile(_modelFile) {}
};

class Level: public Serializable{
    public:
        Level()=default;

        bool static checkIfLevelFileExists(std::string  filename)
        {
                if (fs::exists(filename)) {
                    return true;
                }
                return false;
        }

        void loadMainLevelOfCurrentProject();

        void addRenderable(Renderable *renderable){
            modelFilePaths.push_back(renderable->getName());
            modelTransformations.push_back(&renderable->getModelMatrix());
            renderables->push_back(renderable);
        }

        std::vector<std::string> modelFilePaths;
        std::vector<glm::mat4*> modelTransformations;
        std::vector<Sprites::Text*> textSprites;
        std::vector<Renderable *> *renderables = new std::vector<Renderable *>();
        std::string levelname = "level1";

        std::vector<Camera*> cameras;

        const std::string contentName() override { return levelname;}
        const std::string typeName() const override {return "lvl";}

        std::string GetGuid() {
            return getGUID();
        }
    protected:
        
        void saveContent(fs::path contentFile, std::ostream& os) override;
        void loadContent(fs::path contentFile, std::istream& is) override;

    private:

    static glm::vec3 readVec3(const bs::ptree& parent, const std::string& key) {
        glm::vec3 v;
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