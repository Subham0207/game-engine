#pragma once
#include<Lights/cubemap.hpp>
#include<glm/glm.hpp>
#include<3DModel/mesh.hpp>

class Renderable {
public:
    virtual void draw() = 0;
    virtual void updateFinalBoneMatrix(float deltaTime) {}
    virtual void bindCubeMapTextures(CubeMap *cubeMap) {}
    virtual void updateModelAndViewPosMatrix(glm::vec3 cameraPosition) = 0;
    virtual void imguizmoManipulate(glm::mat4 viewMatrix, glm::mat4 projMatrix) = 0;
    virtual std::vector<Mesh>* getMeshes() = 0;
    virtual glm::mat4& getModelMatrix() = 0;
    virtual std::string getName() = 0;
    virtual ProjectModals::Texture* LoadTexture(std::string filePath, aiTextureType textureType) = 0;
    virtual std::vector<Modals::Material *> getMaterials() = 0;
    virtual void setModelMatrix(glm::mat4 matrix) {}
    virtual void setFileName(std::string filename) {}
    virtual unsigned int getShaderId() const = 0;
    virtual void useAttachedShader() {};
    virtual ~Renderable() = default;
};
