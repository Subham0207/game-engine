#pragma once

#include "Windowing/GameWindow.hpp"

// Separate State Machine tool window.
// Initially a copy of EditorWindow; later you can strip renderer/game tick.
class StateMachineWindow final : public GameWindow {
public:
    // Provide a shared-context parent (optional) to share GL resources.
    explicit StateMachineWindow(GLFWwindow* shareContext = nullptr) : mShareContext(shareContext) {}

    void init() override;
    void tick() override;
    void shutdown() override;

private:
    GLFWwindow* mShareContext = nullptr;
};

