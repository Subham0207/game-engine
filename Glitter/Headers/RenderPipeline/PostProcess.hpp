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
    // Must be called whenever the window framebuffer size changes (GLFW framebuffer size callback).
    void resize(int fbWidth, int fbHeight);
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
    [[nodiscard]] unsigned int getScreenTexture() const { return screenTexture; }

private:
    unsigned int fbo = 0;
    unsigned int rbo = 0;
    unsigned int screenTexture = 0;
    Shader* postProcessShader = nullptr;

    float exposure = 0.3f;

    int mFbWidth = 0;
    int mFbHeight = 0;

    void renderFullscreenTriangle();
};


#endif //GLITTER_POSTPROCESS_HPP