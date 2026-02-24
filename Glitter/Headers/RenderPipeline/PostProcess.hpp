//
// Created by subha on 24-02-2026.
//

#ifndef GLITTER_POSTPROCESS_HPP
#define GLITTER_POSTPROCESS_HPP
#pragma once
#include "LightingPass.hpp"
#include "ShadowPass.hpp"

class PostProcess
{
public:
    PostProcess();
    void draw(
        ShadowPass shadowPass,
        LightingPass lightingPass,
        const std::vector<std::shared_ptr<Renderable>>& renderables,
        Camera* activeCamera,
        Lights* lightSystem,
        CubeMap* cubeMap,
        float deltaTime
        );

    void attachFBO();
    float& getExposure(){return exposure;}

private:
    unsigned int fbo;
    unsigned int rbo;
    unsigned int screenTexture;
    Shader* postProcessShader;

    float exposure = 1.0f;

    void renderFullscreenTriangle();
};


#endif //GLITTER_POSTPROCESS_HPP