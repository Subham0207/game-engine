#pragma once

#include "Windowing/GameWindow.hpp"

// The main editor window (former content of Editor::openEditor loop).
class EditorWindow final : public GameWindow {
public:
    void init() override;
    void tick() override;
    void shutdown() override;
};

