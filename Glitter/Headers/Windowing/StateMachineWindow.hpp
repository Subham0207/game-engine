#pragma once

#include "Windowing/GameWindow.hpp"

#include <GLFW/glfw3.h>

#include <memory>

// Required because this header owns std::unique_ptr<NodeGraph>
#include "NodeGraph/NodeGraph.hpp"

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
    StateMachineWindow(StateMachineWindow&&) noexcept = default;
    StateMachineWindow& operator=(StateMachineWindow&&) noexcept = default;

    void init() override;
    void tickImpl() override;
    void shutdown() override;

private:
    GLFWwindow* mShareContext = nullptr;


    std::unique_ptr<Outliner> mOutliner;
    std::unique_ptr<ProjectAsset::AssetBrowser> mAssetBrowser;
    std::unique_ptr<NodeGraph> mNodeGraph;

    float mSplitRatio = 0.5f;
};

