//
// Created by subha on 24-02-2026.
//

#ifndef GLITTER_SHADOWPASS_HPP
#define GLITTER_SHADOWPASS_HPP
#pragma once
#include "GLFW/glfw3.h"
#include "Lights/light.hpp"


class ShadowPass
{
public:
    ShadowPass(GLFWwindow* window, Lights* lightSystem);
    void draw(float deltaTime, unsigned int FBO) const;
private:
    GLFWwindow* mWindow;
    Lights* mLightSystem;
};


#endif //GLITTER_SHADOWPASS_HPP