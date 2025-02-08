#pragma once
#include "3DModel/model.hpp"
#include "3DModel/Animation/Animator.hpp"
#include "Camera/Camera.hpp"
#include "glm/glm.hpp"
#include <serializeAClass.hpp>
#include <Renderable/renderable.hpp>
#include <Lights/cubemap.hpp>
#include <Helpers/shader.hpp>
#include <3DModel/Skeleton/skeleton.hpp>

class Character: public Renderable
{
public:
    Character(std::string filepath){
        animator = new Animator();
        skeleton = new Skeleton::Skeleton();
        model = new Model(filepath, &skeleton->m_BoneInfoMap, &skeleton->m_BoneCounter);
        skeleton->setup(animator, this->model->getModelMatrix());
    };

    Model* model;
    Animator* animator;
    std::string name;

    void static saveToFile(std::string filename,  Character &character);

    void static loadFromFile(const std::string &filename, Character &character);

    auto& GetBoneInfoMap() { return skeleton->m_BoneInfoMap; }
    int& GetBoneCount() { return skeleton->m_BoneCounter; }

    void updateFinalBoneMatrix(float deltatime);

    void draw(float deltaTime, Camera* camera, Lights* lights, CubeMap* cubeMap) override;

    void useAttachedShader() override;

    void imguizmoManipulate(glm::mat4 viewMatrix, glm::mat4 projMatrix) override
    {
        model->imguizmoManipulate(viewMatrix, projMatrix);
    }

    std::vector<Mesh>* getMeshes() override{
        return model->getMeshes();
    };
    glm::mat4& getModelMatrix() override{
        return model->getModelMatrix();
    };

    std::string getName() override{
        return model->getName();
    }

    ProjectModals::Texture* LoadTexture(std::string filePath, aiTextureType textureType) override{
        return model->LoadTexture(filePath, textureType);
    }

    std::vector<Modals::Material *> getMaterials() override
    {
        return model->getMaterials();
    }

    Skeleton::Skeleton* skeleton;
private:
    Camera* camera;

    glm::mat4 transformation;

    void setFinalBoneMatrix(int boneIndex, glm::mat4 transform);


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