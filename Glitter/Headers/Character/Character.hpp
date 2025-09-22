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
#include <Helpers/vertexBoneDataHelper.hpp>
#include <Controls/PlayerController.hpp>
#include <Controls/statemachine.hpp>
#include <Physics/capsule.hpp>

class Character: public Renderable
{
public:
    Character(std::string filepath);

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
    Controls::PlayerController* playerController;
    
    Controls::StateMachine* animStateMachine;

    Physics::Capsule* capsuleCollider;
    glm::vec3 capsuleColliderPosRelative;

    void physicsUpdate() override;
    void syncTransformationToPhysicsEntity() override;
    float cameraHeight = 7;
    float cameraDistance = 16;

    virtual glm::vec3 GetPosition()
    {
        return model->GetPosition();
    }

    virtual glm::vec3 GetScale()
    {
        return model->GetScale();
    }

    virtual glm::quat GetRot()
    {
        return model->GetRot();
    }

    virtual std::string GetGuid() override {
        return "random_guid";
    }

private:
    Camera* camera;

    glm::mat4 transformation;

    glm::vec3 forwardVector;
    glm::vec3 rightVector;

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