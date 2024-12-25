#pragma once
#include<Lights/cubemap.hpp>
#include<glm/glm.hpp>

class Renderable {
public:
    virtual void draw() = 0;
    virtual void updateFinalBoneMatrix(float deltaTime) {}
    virtual void bindCubeMapTextures(CubeMap *cubeMap) {}
    virtual void updateModelAndViewPosMatrix(glm::vec3 cameraPosition) {}
    virtual unsigned int getShaderId() const = 0;
    virtual void useAttachedShader() {};
    virtual ~Renderable() = default;
};
