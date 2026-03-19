//
// Created by subha on 18-03-2026.
//

#pragma once

#include <filesystem>

#include <GLFW/glfw3.h>

#include <RenderPipeline/LightingPass.hpp>
#include <RenderPipeline/PostProcess.hpp>

class Lights;
class Camera;
class CubeMap;
class NodeGraph;
class InputHandler;

namespace UI
{
    // Opens a dedicated editor window hosting a split view:
    //  - Left: a basic PBR scene viewport (render pipeline output).
    //  - Right: NodeGraph-based state machine editor UI.
    //
    // Note: This class assumes the engine has already initialized GLAD,
    // created an ImGui context, and set up the main EngineState.
    class StateMachineEditor
    {
    public:
        explicit StateMachineEditor(std::filesystem::path engineRoot);
        ~StateMachineEditor();

        // Creates the GLFW window and initializes per-window backends.
        // Returns false on failure (window not created).
        bool open();

        // Call once per engine frame. Returns false when user closes the window.
        bool tick(
            const std::vector<std::shared_ptr<Renderable>>& renderables,
            Camera* activeCamera,
            Lights* lights,
            CubeMap* cubeMap,
            float deltaTime);

        void close();

        [[nodiscard]] bool isOpen() const { return mWindow != nullptr && !glfwWindowShouldClose(mWindow); }
        [[nodiscard]] GLFWwindow* window() const { return mWindow; }

    private:
        std::filesystem::path mEngineRoot;

        GLFWwindow* mWindow = nullptr;
        NodeGraph* mNodeGraph = nullptr;

        // Pipeline pieces: keep local to this window so we can render a basic scene.
        LightingPass mLightingPass{};
        PostProcess mPostProcess{};

        // Simple UI settings
        float mSplitRatio = 0.5f;

        void beginFrame();
        void endFrame();

        void drawDockspaceAndSplitUI();
        void drawLeftSceneViewport(
            const std::vector<std::shared_ptr<Renderable>>& renderables,
            Camera* activeCamera,
            Lights* lights,
            CubeMap* cubeMap,
            float deltaTime,
            int viewportX,
            int viewportY,
            int viewportW,
            int viewportH);

        void drawRightNodeGraph();
    };
}



