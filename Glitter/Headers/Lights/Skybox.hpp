//
// Created by subha on 12-03-2026.
//

#ifndef GLITTER_SKYBOX_HPP
#define GLITTER_SKYBOX_HPP

#include "GLFW/glfw3.h"
#include <filesystem>

#include "glm/glm.hpp"

namespace fs = std::filesystem;

class CubeMap;
class Shader;
namespace Lighting
{
    class Skybox
    {
        public:
            Skybox(fs::path basePath, GLFWwindow* window);
            [[nodiscard]] CubeMap* getCubeMap() const;

            void Draw(glm::mat4& view, glm::mat4& proj) const;
        private:
            CubeMap* cubeMap;
            Shader* backgroundShader;
    };

}


#endif //GLITTER_SKYBOX_HPP