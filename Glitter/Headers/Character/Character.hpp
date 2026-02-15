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
#include <Controls/statemachine.hpp>
#include <Physics/capsule.hpp>
#include <Serializable.hpp>
#include <Controls/Empty.hpp>

#include "CharacterPrefabConfig.hpp"

namespace Controls
{
    class PlayerController;
}

/*
 * Character Entity prefab structure ( Used for serialization )
    {
        "classId": "WarriorCharacter",
        "model_guid": "GUID", // static file
        "skeleton_guid": "GUID", // static file
        "statemachine": {
            "classId": "warriorStateMachine" // states creation, switching on condition all live in this file
        },
    }

    Nothing scriptable or anything that can be saved to static file in camera, capsule collider, playerController.
    So we can create them in onStart() of WarriorCharacter derived class

    When you import a character the editor UI will ask you to choose the type.
    If you choose just model. It will create a .model file. ( which is not scriptable yet ). Click save it will not create Entity obj.
    Choose Character. This will reveal additional option: Choose flavor of Character. Click save it will create entity obj similar to above.
 */

class Character: public Renderable, public Serializable
{
public:
    Character() = default;
    Character(std::string filepath);
    ~Character() override;

    std::string GetClassId() const override { return "Character"; }

    Model* model;
    std::string model_guid;

    Animator* animator;
    std::string filename;

    virtual void onStart(){};
    virtual void onTick(){};
    virtual void onDestroy(){};

    void static saveToFile(std::string filename,  Character &character);
    void static loadPrefabIntoActiveLevel(const CharacterPrefabConfig& characterPrefab);

    void loadStateMachine(std::string stateMachine_guid);
    void deleteStateMachine();

    void static loadFromFile(const std::string &filename, Character &character);

    auto& GetBoneInfoMap() { return skeleton->m_BoneInfoMap; }
    int& GetBoneCount() { return skeleton->m_BoneCounter; }

    void updateFinalBoneMatrix(float deltatime);

    void draw(float deltaTime, Camera* camera, Lights* lights, CubeMap* cubeMap) override;
    void drawGeometryOnly() override;

    virtual std::vector<ProjectModals::Vertex> GetWorldVertices() override;
    virtual std::vector<unsigned int> GetIndices() override;

    virtual ModelType getModelType() override {return model->modeltype;}

    void useAttachedShader() override;

    void imguizmoManipulate(glm::mat4 viewMatrix, glm::mat4 projMatrix) override;

    std::vector<Mesh>* getMeshes() override{
        return model->getMeshes();
    };
    glm::mat4& getModelMatrix() override{
        return empty.getWorldTransform();
    };

    void setModelMatrix(glm::mat4 matrix) override{
        empty.setWorldTransform(matrix);
    };

    std::string getName() override{
        return filename;
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
    
    std::shared_ptr<Controls::Controller> controller;
    
    std::shared_ptr<Controls::StateMachine> animStateMachine;
    std::string animStateMachine_guid;

    Physics::Capsule* capsuleCollider;
    glm::vec3 modelRelativePosition;

    //Deprecated
    glm::vec3 capsuleColliderPosRelative;

    void physicsUpdate() override;
    void syncTransformationToPhysicsEntity() override;
    float cameraHeight = 7;
    float cameraDistance = 16;

    glm::vec3 GetPosition() override
    {
        return empty.getWorldPosition();
    }

    glm::vec3 GetScale() override
    {
        return empty.getScale();
    }

    glm::quat GetRot() override
    {
        return empty.getWorldRotation();
    }

    void setWorldTransform(const glm::mat4& transform)
    {
        empty.setWorldTransform(transform);
    }

    void setWorldTransform(const glm::vec3& position, const glm::quat& rotation)
    {
        empty.setWorldTransform(position, rotation);
    }

    void setScale(glm::vec3 scale)
    {
        empty.setWorldTransform(
            GetPosition(),
            GetRot(),
            scale
            );
    }

    std::string GetGuid() override {
        return getAssetId();
    }

    void setIsSelected(bool isSelected) override
    {
        this->isSelected = isSelected;
    };
    bool getIsSelected() override
    {
        return isSelected;
    };
    bool isSelected = false;

    void generateInstanceGuid()
    {
        generate_instance_guid();
    }

    [[nodiscard]] glm::vec3 movement_offset() const
    {
        return movementOffset;
    }

    void set_movement_offset(const glm::vec3& movement_offset)
    {
        movementOffset = movement_offset;
    }

    [[nodiscard]] glm::quat rotation_offset() const
    {
        return rotationOffset;
    }

    void set_rotation_offset(const glm::quat& rotation_offset)
    {
        rotationOffset = rotation_offset;
    }

    Camera* camera;
    float smoothAngle(float current, float target, float t);
protected:
    virtual const std::string typeName() const override {return "character"; }
    virtual const std::string contentName() override {return filename; }

    virtual void saveContent(fs::path contentFileLocation, std::ostream& os) override;
    virtual void loadContent(fs::path contentFileLocation, std::istream& is) override;

private:
    glm::vec3 movementOffset{};
    glm::quat rotationOffset = glm::identity<glm::quat>();

    Controls::Empty empty{};

    //deprecated
    glm::mat4 transformation;

    glm::vec3 forwardVector;
    glm::vec3 rightVector;

    bool started = false;

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