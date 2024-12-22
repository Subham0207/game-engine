#pragma once
#include "3DModel/model.hpp"
#include "3DModel/Animation/Animator.hpp"
#include "Camera/Camera.hpp"
#include "glm/glm.hpp"
#include <serializeAClass.hpp>

class Character
{
public:
    Character(std::string filepath){
        animator = new Animator();
        model = new Model(filepath, &m_BoneInfoMap, &m_BoneCounter);
    };

    Model* model;
    Animator* animator;
    std::string name;

    void static saveToFile(std::string filename,  Character &character);

    void static loadFromFile(const std::string &filename, Character &character);

    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }

    void updateFinalBoneMatrix(float deltatime);

private:
    Camera* camera;

    glm::mat4 transformation;

    std::map<std::string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

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