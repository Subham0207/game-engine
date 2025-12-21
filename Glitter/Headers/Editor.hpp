//
// Created by subha on 20-12-2025.
//

#ifndef GLITTER_EDITOR_HPP
#define GLITTER_EDITOR_HPP
#include "GLFW/glfw3.h"

class Editor
{
public:
    int openEditor();
private:
    GLFWwindow* mWindow = nullptr;
};


#endif //GLITTER_EDITOR_HPP