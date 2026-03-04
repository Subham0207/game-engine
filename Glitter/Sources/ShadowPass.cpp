//
// Created by subha on 24-02-2026.
//

#include "../Headers/RenderPipeline/ShadowPass.hpp"

#include <Profiler.hpp>

ShadowPass::ShadowPass(GLFWwindow* window, Lights* lightSystem)
{
    mWindow = window;
    mLightSystem = lightSystem;
}

void ShadowPass::draw(const float deltaTime, unsigned int FBO) const
{
    ZoneScopedN("ShadowPassDraw");
    for(auto& light : mLightSystem->directionalLights)
        light.evaluateShadowMap(mWindow, deltaTime, FBO);

    for(auto& light : mLightSystem->spotLights)
        light.evaluateShadowMap(mWindow, deltaTime, FBO);

    for(auto& light : mLightSystem->pointLights)
        light.evaluateShadowMap(mWindow, deltaTime, FBO);
}
