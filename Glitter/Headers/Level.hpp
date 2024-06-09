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

            std::ofstream ofs(filename);
            boost::archive::text_oarchive oa(ofs);
            oa << lvl;
        }

        Level static loadFromFile(const std::string &filename, Level &lvl) {
            std::ifstream ifs(filename);
            boost::archive::text_iarchive ia(ifs);
            ia >> lvl;
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
            models->push_back(model);
        }
        std::vector<Model *> *models = new std::vector<Model *>();
        std::string levelname = "level1";
    private:

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & models;
            ar & levelname;
        }
};