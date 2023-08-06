#include "Camera.hpp"
#include <glad/glad.h>

Camera::Camera()
{
}

void Camera::updateMVP(unsigned int shader)
{
    viewLoc = glGetUniformLocation(shader, "view");
    projectionLoc = glGetUniformLocation(shader, "projection");
    setupView();
    setupProjection();
}

glm::vec3 Camera::getPosition()
{
    return cameraPos;
}

glm::vec3 Camera::getFront()
{
    return cameraFront;
}

void Camera::setupView()
{
    glm::mat4 view;
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void Camera::setupProjection()
{
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}
