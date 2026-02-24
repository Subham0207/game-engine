//
// Created by subha on 24-02-2026.
//

#include "../Headers/RenderPipeline/PostProcess.hpp"

#include <filesystem>

#include "glad/glad.h"
#include <Helpers/glitter.hpp>

#include "EngineState.hpp"
namespace fs = std::filesystem;

PostProcess::PostProcess()
{
    exposure = 1.0f;
    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    auto vsPath = engineFSPath / "Shaders/PostProcessing/PostProcess.vert";
    auto fsPath = engineFSPath / "Shaders/PostProcessing/PostProcess.frag";
    postProcessShader = new Shader(vsPath.u8string().c_str(), fsPath.u8string().c_str());

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // 1. Create the texture (The "screenTexture")
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    // Use GL_RGB16F for HDR (High Dynamic Range) support!
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, (float)mWidth, (float)mHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 2. Attach it to the Framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (float)mWidth, (float)mHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Frame buffer error: " << fboStatus << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcess::draw(
    ShadowPass shadowPass,
    LightingPass lightingPass,
    const std::vector<std::shared_ptr<Renderable>>& renderables,
    Camera* activeCamera,
    Lights* lightSystem,
    CubeMap* cubeMap,
    float deltaTime
    )
{
    // Shadows don't always need to be in this FBO,
    // but they must be done before Lighting.
    shadowPass.draw(deltaTime, fbo);

    lightingPass.draw(renderables, activeCamera, lightSystem, cubeMap, deltaTime);

    // --- STEP 2: RENDER THE FBO TO THE SCREEN ---
    // Now we switch to the "Actual Monitor" (0)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    postProcessShader->use();

    // Bind the texture we just drew to in Step 1
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->screenTexture);
    postProcessShader->setInt("screenTexture", 0);
    postProcessShader->setFloat("exposure", exposure);

    // Draw the fullscreen triangle/quad
    glDisable(GL_DEPTH_TEST);
    renderFullscreenTriangle();
    glEnable(GL_DEPTH_TEST);
}

void PostProcess::attachFBO()
{
    // --- STEP 1: RENDER THE SCENE TO THE FBO ---
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcess::renderFullscreenTriangle() {
    static unsigned int dummyVAO = 0;
    if (dummyVAO == 0) {
        // We create one empty VAO and never fill it with buffers.
        // The shader uses gl_VertexID to "invent" the geometry.
        glGenVertexArrays(1, &dummyVAO);
    }

    glBindVertexArray(dummyVAO);
    // Draw 3 vertices. This triggers the Vertex Shader 3 times.
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
