//
// Created by subha on 17-01-2026.
//

#include "../Headers/Camera/FlyCam.hpp"

FlyCam::FlyCam(const std::string& name)
{
    this->cameraName = name;
    yaw = -90.0f;
    pitch = 0.0f;
}

void FlyCam::onMouseMove(const MouseMoveEvent& e)
{
    Camera::onMouseMove(e);

    if(e.mouseState == GLFW_CURSOR_NORMAL)
        return;

    moveCamera(e.xOffset, e.yOffset);
}

void FlyCam::moveCamera(double xOffset, double yOffset)
{
    const float sensitivity = 0.05f;

    yaw += (xOffset * sensitivity);
    pitch += (yOffset * sensitivity);

    if (pitch > 89.0f)
    pitch = 89.0f;
    if (pitch < -89.0f)
    pitch = -89.0f;


    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}
