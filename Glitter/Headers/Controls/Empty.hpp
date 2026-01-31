//
// Created by subha on 31-01-2026.
//

#ifndef GLITTER_EMPTY_HPP
#define GLITTER_EMPTY_HPP

#include <glm/glm.hpp>
#include "glm/ext/matrix_transform.hpp"

namespace Controls
{
    class Empty
    {
    public:
        glm::mat4& getWorldTransform();

        void setWorldTransform(glm::mat4 matrix);

        void setWorldTransform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

        void setWorldTransform(const glm::vec3& position, const glm::quat& rotation);

        glm::vec3 getWorldPosition();

        glm::vec3 getScale();

        glm::quat getWorldRotation();
    private:
        glm::mat4 transform = glm::identity<glm::mat4>();
    };
}


#endif //GLITTER_EMPTY_HPP