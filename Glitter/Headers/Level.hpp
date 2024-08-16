#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/access.hpp>

#include <model.hpp>

namespace fs = std::filesystem;

struct LevelDetails
{
    std::string modelFilePath;
    Model* modelFile;

    LevelDetails(std::string _modelFilePath, Model* _modelFile)
    : modelFilePath(_modelFilePath), modelFile(_modelFile) {}
};

class Level{
    public:
        Level()=default;

        void static saveToFile(const std::string &filename, const Level &lvl) {
            fs::path dir = fs::path(filename).parent_path();
            if (dir.empty()) {
                // Set the directory to the current working directory
                dir = fs::current_path();
            }
            if (!fs::exists(dir)) {
                if (!fs::create_directories(dir)) {
                    std::cerr << "Failed to create directories: " << dir << std::endl;
                    return;
                }
            }

            for (size_t i = 0; i < lvl.modelFilePaths.size(); i++)
            {
                std::cout << "Saving...." << std::endl;
                std::cout << "mat[0]: " << lvl.modelTransformations[i][0][0].x <<" "<< lvl.modelTransformations[i][0][0].y <<" "<< lvl.modelTransformations[i][0][0].z << " " << lvl.modelTransformations[i][0][0].w << std::endl;
                std::cout << "mat[0]: " << lvl.modelTransformations[i][0][1].x <<" "<< lvl.modelTransformations[i][0][1].y <<" "<< lvl.modelTransformations[i][0][1].z << " " << lvl.modelTransformations[i][0][1].w << std::endl;
                std::cout << "mat[0]: " << lvl.modelTransformations[i][0][2].x <<" "<< lvl.modelTransformations[i][0][2].y <<" "<< lvl.modelTransformations[i][0][2].z << " " << lvl.modelTransformations[i][0][2].w << std::endl;
                std::cout << "mat[0]: " << lvl.modelTransformations[i][0][3].x <<" "<< lvl.modelTransformations[i][0][3].y <<" "<< lvl.modelTransformations[i][0][3].z << " " << lvl.modelTransformations[i][0][3].w << std::endl;
            }


            std::ofstream ofs(filename);
            boost::archive::text_oarchive oa(ofs);
            oa << lvl;
        }

        Level static loadFromFile(const std::string &filename, Level &lvl) {
            std::ifstream ifs(filename);
            boost::archive::text_iarchive ia(ifs);
            ia >> lvl;
            for (size_t i = 0; i < lvl.modelFilePaths.size(); i++)
            {
                auto model = new Model();
                Model::loadFromFile(lvl.modelFilePaths[i], *model);// this also sets modelMatrix to identityMatrix
                model->model = *lvl.modelTransformations[i]; // Set modelMatrix
                lvl.models->push_back(model);
                delete lvl.modelTransformations[i];                 
                lvl.modelTransformations[i] = &model->model;//point lvl's ModelMatrix back to Model's Matrix;
            }

            lvl.camera->cameraFront = *lvl.cameraFront;
            lvl.camera->cameraPos = *lvl.cameraPos;
            lvl.camera->cameraUp = *lvl.cameraUp;

            delete lvl.cameraFront;
            delete lvl.cameraPos;
            delete lvl.cameraUp;

            lvl.cameraFront = &lvl.camera->cameraFront;
            lvl.cameraPos = &lvl.camera->cameraPos;
            lvl.cameraUp = &lvl.camera->cameraUp;

            return lvl;
        }

        bool static checkIfLevelFileExists(std::string  filename)
        {
                if (fs::exists(filename)) {
                    return true;
                }
                return false;
        }

        void addModel(Model *model){
            modelFilePaths.push_back(model->getName());
            modelTransformations.push_back(&model->model);
            models->push_back(model);
        }
        std::vector<std::string> modelFilePaths;
        std::vector<glm::mat4*> modelTransformations;
        std::vector<Model *> *models = new std::vector<Model *>();
        std::string levelname = "level1";

        glm::vec3* cameraPos;
    	glm::vec3* cameraFront;
	    glm::vec3* cameraUp;

        Camera* camera;

    private:

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & modelFilePaths;
            ar & modelTransformations;
            ar & levelname;

            ar & cameraPos;
            ar & cameraFront;
            ar & cameraUp;
        }
};