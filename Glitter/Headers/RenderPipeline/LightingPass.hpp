//
// Created by subha on 24-02-2026.
//

#ifndef GLITTER_LIGHTINGPASS_HPP
#define GLITTER_LIGHTINGPASS_HPP
#pragma once
#include "Renderable/renderable.hpp"
#include <vector>

class Camera;
class LightingPass
{
public:
    void draw(
        const std::vector<std::shared_ptr<Renderable>>& renderables,
        Camera* activeCamera,
        Lights* lightSystem,
        CubeMap* cubeMap,
        float deltaTime
        );
};


#endif //GLITTER_LIGHTINGPASS_HPP