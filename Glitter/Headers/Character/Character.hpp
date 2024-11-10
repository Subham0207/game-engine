#pragma once
#include "3DModel/model.hpp"
#include "3DModel/Animation/Animator.hpp"
#include "Camera/Camera.hpp"
#include "glm/glm.hpp"
#include <serializeAClass.hpp>

class Character
{
public:
    Character(){
        animator = new Animator();
    };

    Model* model;
    Animator* animator;
    std::string name;

    void static saveToFile(std::string filename,  Character &character);

    void static loadFromFile(const std::string &filename, Character &character);

private:
    Camera* camera;

    glm::mat4 transformation;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & model;
        ar & animator;
        ar & camera;
        ar & transformation;
        ar & name;
    }
};