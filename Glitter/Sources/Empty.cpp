//
// Created by subha on 31-01-2026.
//

#include "../Headers/Controls/Empty.hpp"
#include "glm/gtc/quaternion.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

namespace Controls
{
    glm::mat4& Empty::getWorldTransform()
    {
        return transform;
    }

    void Empty::setWorldTransform(const glm::mat4 matrix)
    {
        this->transform = matrix;
    }

    void Empty::setWorldTransform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
    {
        const glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        const glm::mat4 R = glm::toMat4(rotation);
        const glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

        transform = T * R * S;
    }

    void Empty::setWorldTransform(const glm::vec3& position, const glm::quat& rotation)
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 R = glm::toMat4(rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), getScale());

        transform = T * R * S;
    }

    glm::vec3 Empty::getWorldPosition()
    {
        return transform[3];
    }

    glm::vec3 Empty::getScale()
    {
        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(transform[0])); // X column
        scale.y = glm::length(glm::vec3(transform[1])); // Y column
        scale.z = glm::length(glm::vec3(transform[2])); // Z column
        return scale;
    }

    glm::quat Empty::getWorldRotation()
    {
        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(transform[0]));
        scale.y = glm::length(glm::vec3(transform[1]));
        scale.z = glm::length(glm::vec3(transform[2]));

        glm::mat4 rotMat = transform;

        // Remove scale from rotation matrix
        rotMat[0] /= scale.x;
        rotMat[1] /= scale.y;
        rotMat[2] /= scale.z;

        return glm::quat_cast(rotMat);
    }
}