#pragma once

#include <memory>

#include "Windowing/AnimationEditorWindow.hpp"

class EditorAnimation
{
public:
    EditorAnimation();
    int openEditor();

private:
    std::unique_ptr<AnimationEditorWindow> mWindow;
};
