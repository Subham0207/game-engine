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
#include <Serializable.hpp>

class Character: public Renderable, public Serializable
{
public:
    Character() = default;
    Character(std::string filepath);
    ~Character();

    Model* model;
    std::string model_guid;

    Animator* animator;
    std::string filename;

    void static saveToFile(std::string filename,  Character &character);

    void loadStateMachine(std::string stateMachine_guid);
    void deleteStateMachine();

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

    void setModelMatrix(glm::mat4 matrix) override{
       model->setModelMatrix(matrix);
    };

    std::string getName() override{
        return model->getName();
    }

    std::shared_ptr<ProjectModals::Texture> LoadTexture(std::string filePath, aiTextureType textureType) override{
        return model->LoadTexture(filePath, textureType);
    }

    std::vector<std::shared_ptr<Modals::Material>> getMaterials() override
    {
        return model->getMaterials();
    }

    Skeleton::Skeleton* skeleton;
    std::string skeleton_guid;
    
    Controls::PlayerController* playerController;
    
    Controls::StateMachine* animStateMachine;
    std::string animStateMachine_guid;

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
        return getGUID();
    }

protected:
    virtual const std::string typeName() const override {return "character"; }
    virtual const std::string contentName() override {return filename; }

    virtual void saveContent(fs::path contentFileLocation, std::ostream& os) override;
    virtual void loadContent(fs::path contentFileLocation, std::istream& is) override;

private:
    Camera* camera;

    glm::mat4 transformation;

    glm::vec3 forwardVector;
    glm::vec3 rightVector;

    void setFinalBoneMatrix(int boneIndex, glm::mat4 transform);


    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & capsuleCollider;
        ar & capsuleColliderPosRelative;
        ar & camera;
        ar & transformation;
        ar & filename;
        ar & model_guid;
        ar & animStateMachine_guid;
        ar & skeleton_guid;
    }
};