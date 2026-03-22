#pragma once

#include "Windowing/GameWindow.hpp"

#include <GLFW/glfw3.h>

#include <memory>

// Required because this header owns std::unique_ptr<NodeGraph>
#include "NodeGraph/NodeGraph.hpp"

#include "RenderPipeline/SceneViewport.hpp"

#include "Lights/Skybox.hpp"
#include "Lights/light.hpp"

#include "Level/Level.hpp"

#include "Event/EventBus.hpp"

class EventQueue;
struct InputContext;
class InputHandler;

class Outliner;
namespace ProjectAsset { class AssetBrowser; }
// NodeGraph included above

// Separate State Machine tool window.
// Initially a copy of EditorWindow; later you can strip renderer/game tick.
class StateMachineWindow final : public GameWindow {
public:
    // Provide a shared-context parent (optional) to share GL resources.
    explicit StateMachineWindow(GLFWwindow* shareContext);

    StateMachineWindow(const StateMachineWindow&) = delete;
    StateMachineWindow& operator=(const StateMachineWindow&) = delete;
    // This type owns unique_ptrs and also stores an EventBus; disable moves for simplicity.
    StateMachineWindow(StateMachineWindow&&) noexcept = delete;
    StateMachineWindow& operator=(StateMachineWindow&&) noexcept = delete;

    void init() override;
    void tickImpl() override;
    void shutdown() override;

private:
    GLFWwindow* mShareContext = nullptr;

    // Local bus for this window so mouse-look can drive its own camera.
    EventBus mWindowBus;


    std::unique_ptr<SceneViewport> mSceneViewport;

    // Inline scene resources for this tool window (kept local for now).
    // This matches EditorWindow's pattern so we can render immediately without depending
    // on EditorWindow ownership.
    std::unique_ptr<Level> mPreviewLevel;
    std::unique_ptr<Lights> mLights;
    std::unique_ptr<Lighting::Skybox> mSkyBox;

    std::unique_ptr<NodeGraph> mNodeGraph;
};

