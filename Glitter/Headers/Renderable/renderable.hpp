#pragma once
#include<Lights/cubemap.hpp>
#include<Lights/light.hpp>
#include<glm/glm.hpp>
#include<3DModel/mesh.hpp>
#include<Camera/Camera.hpp>

class Renderable {
public:
    virtual void draw(float deltaTime, Camera* camera, Lights* lights, CubeMap* cubeMap) = 0;
    virtual void drawGeometryOnly() = 0;
    virtual void imguizmoManipulate(glm::mat4 viewMatrix, glm::mat4 projMatrix) = 0;
    virtual std::vector<Mesh>* getMeshes() = 0;
    virtual glm::mat4& getModelMatrix() = 0;
    virtual glm::vec3 GetPosition() = 0;
    virtual glm::quat GetRot() = 0;
    virtual glm::vec3 GetScale() = 0;
    virtual std::string getName() = 0;
    virtual std::shared_ptr<ProjectModals::Texture> LoadTexture(std::string filePath, aiTextureType textureType) = 0;
    virtual std::vector<std::shared_ptr<Modals::Material>> getMaterials() = 0;
    virtual void setModelMatrix(glm::mat4 matrix) {}
    virtual void setFileName(std::string filename) {}
    virtual void useAttachedShader() {};
    virtual ~Renderable() = default;
    virtual bool ShouldRender() {return true;}; // implment this method and return false for debug meshes.
    virtual void setIsSelected(bool isSelected) = 0;
    virtual bool getIsSelected() = 0;

    virtual void physicsUpdate(){};
    virtual void syncTransformationToPhysicsEntity(){};

    virtual std::string GetGuid() = 0;
};
