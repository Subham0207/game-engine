#include "RenderPipeline/SceneViewport.hpp"

#include "RenderPipeline/ShadowPass.hpp"
#include "RenderPipeline/LightingPass.hpp"
#include "RenderPipeline/PostProcess.hpp"

#include "Camera/Camera.hpp"
#include "Lights/light.hpp"

#include <memory>
#include <vector>

#include "Renderable/renderable.hpp"

SceneViewport::~SceneViewport() = default;

void SceneViewport::init(GLFWwindow* window, Lights* lights)
{
    mWindow = window;

    // Pipeline objects are lightweight wrappers around GL resources.
    // We keep them per-window to avoid FBO/viewport state collisions.
    mShadowPass = std::make_unique<ShadowPass>(mWindow, lights);
    mLightingPass = std::make_unique<LightingPass>();
    mPostProcess = std::make_unique<PostProcess>();
}

void SceneViewport::resize(int fbWidth, int fbHeight)
{
    if (!mPostProcess)
        return;

    // PostProcess internally caches the last size and will recreate attachments as needed.
    mPostProcess->resize(fbWidth, fbHeight);
}

void SceneViewport::render(
    const std::vector<std::shared_ptr<Renderable>>& renderables,
    Camera* camera,
    Lights* lights,
    CubeMap* cubeMap,
    float deltaTime)
{
    if (!mPostProcess || !mShadowPass || !mLightingPass || !camera)
        return;

    mPostProcess->attachFBO();

    mPostProcess->draw(
        *mShadowPass,
        *mLightingPass,
        renderables,
        camera,
        lights,
        cubeMap,
        deltaTime);
}

unsigned int SceneViewport::textureId() const
{
    return mPostProcess ? mPostProcess->getScreenTexture() : 0u;
}



