//
// Created by subha on 12-03-2026.
//

#ifndef GLITTER_RAYCAST_HPP
#define GLITTER_RAYCAST_HPP

#include <filesystem>

#include "glm/glm.hpp"
#include "Windowing/FrameContext.hpp"
namespace fs = std::filesystem;

class Shader;
class Outliner;
class Camera;
class Level;
class InputHandler;
namespace Debug
{
    class Raycast
    {
    public:
        Raycast(fs::path basePath);
        void HandleSelection(const FrameContext& frameCtx, Outliner* outliner, Camera* activeCamera, const Level* level, InputHandler* input);
    private:
        Shader *rayCastshader;
        glm::vec3 rayOrigin, rayDir;
    };

}


#endif //GLITTER_RAYCAST_HPP