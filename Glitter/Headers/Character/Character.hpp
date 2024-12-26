#pragma once
#include "3DModel/model.hpp"
#include "3DModel/Animation/Animator.hpp"
#include "Camera/Camera.hpp"
#include "glm/glm.hpp"
#include <serializeAClass.hpp>
#include <Renderable/renderable.hpp>
#include <Lights/cubemap.hpp>
#include <Helpers/shader.hpp>

class Character: public Renderable
{
public:
    Character(std::string filepath){
        animator = new Animator();
        model = new Model(filepath, &m_BoneInfoMap, &m_BoneCounter);
        bonesShader = new Shader("E:/OpenGL/Glitter/Glitter/Shaders/boneShader.vert", "E:/OpenGL/Glitter/Glitter/Shaders/boneShader.frag");
        auto transforms = animator->GetFinalBoneMatrices();
        for (int i = 0; i < m_BoneInfoMap.size(); ++i)
        {
            extractBonePositions(i, transforms[i]);
        }
        setupBoneBuffersOnGPU();
    };

    Model* model;
    Animator* animator;
    std::string name;

    void static saveToFile(std::string filename,  Character &character);

    void static loadFromFile(const std::string &filename, Character &character);

    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }

    void updateFinalBoneMatrix(float deltatime);

    void draw() override;

    void bindCubeMapTextures(CubeMap *cubemap) override;

    void useAttachedShader() override;

    unsigned int getShaderId() const override;

    void updateModelAndViewPosMatrix(Camera* camera) override
    {
        model->updateModelAndViewPosMatrix(camera);

        glm::mat4 projection = camera->projectionMatrix();
        glm::mat4 view = camera->viewMatrix();

        bonesShader->use();
        bonesShader->setMat4("projection", projection);
        bonesShader->setMat4("view", view);
        bonesShader->setMat4("model", model->getModelMatrix());

        model->shader->use();
    }

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

private:
    Camera* camera;

    glm::mat4 transformation;

    std::map<std::string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;
    std::vector<glm::vec3> bonePositions;
    unsigned int bonesVAO;
    unsigned int bonesVBO;


    Shader* bonesShader;
    void setFinalBoneMatrix(int boneIndex, glm::mat4 transform);
    void extractBonePositions(int boneIndex, glm::mat4 transform);
    void setupBoneBuffersOnGPU();
    void renderBones();

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