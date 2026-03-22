#pragma once

#include <GLFW/glfw3.h>

#include <memory>
#include <vector>

// Required: SceneViewport owns std::unique_ptr<PostProcess> and may be destroyed
// in translation units that only include this header.
#include "RenderPipeline/PostProcess.hpp"

class Camera;
class CubeMap;
class Lights;
class LightingPass;
class ShadowPass;
namespace Lighting { class Skybox; }

namespace std { template <class T> class shared_ptr; }
class Renderable;

// A reusable per-window "scene viewport" renderer.
// Owns a minimal render pipeline (shadow + lighting + postprocess) and renders
// the active level into an offscreen texture (PostProcess::screenTexture).
//
// Intended to be embedded in multiple windows (EditorWindow, StateMachineWindow).
class SceneViewport
{
public:
    SceneViewport() = default;
    SceneViewport(const SceneViewport&) = delete;
    SceneViewport& operator=(const SceneViewport&) = delete;
    SceneViewport(SceneViewport&&) noexcept = default;
    SceneViewport& operator=(SceneViewport&&) noexcept = default;
    ~SceneViewport();

    void init(GLFWwindow* window, Lights* lights);

    // Desired render target size in pixels.
    void resize(int fbWidth, int fbHeight);

    void render(
        const std::vector<std::shared_ptr<Renderable>>& renderables,
        Camera* camera,
        Lights* lights,
        Lighting::Skybox* skybox,
        float deltaTime);

    [[nodiscard]] unsigned int textureId() const;

private:
    GLFWwindow* mWindow = nullptr; // non-owning

    std::unique_ptr<ShadowPass> mShadowPass;
    std::unique_ptr<LightingPass> mLightingPass;
    std::unique_ptr<PostProcess> mPostProcess;
};

