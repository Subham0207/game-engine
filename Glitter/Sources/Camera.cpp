#include "Camera/Camera.hpp"
#include <glad/glad.h>
#include <Controls/Input.hpp>

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
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void Camera::setupProjection()
{
    projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void Camera::FrameModel(const aiAABB& boundingBox) {
    // Calculate the center of the bounding box
    glm::vec3 center = 0.5f * (glm::vec3(boundingBox.mMax.x, boundingBox.mMax.y, boundingBox.mMax.z) +
                               glm::vec3(boundingBox.mMin.x, boundingBox.mMin.y, boundingBox.mMin.z));

    // Calculate the size of the bounding box (diagonal length)
    glm::vec3 size = glm::vec3(boundingBox.mMax.x - boundingBox.mMin.x,
                               boundingBox.mMax.y - boundingBox.mMin.y,
                               boundingBox.mMax.z - boundingBox.mMin.z);
    float maxSize = glm::length(size);

    // Calculate new camera position. This is a simplistic approach and might need adjustment.
    // Assumes the camera looks towards the negative Z axis initially.
    float distance = maxSize / (2.0f * tan(glm::radians(fov) / 2.0f));
    glm::vec3 newCameraPos = center - glm::vec3(0, 0, distance);

    // Update camera position and look at the center of the model
    cameraPos = newCameraPos;
    lookAt(center);
}

void Camera::lookAt(glm::vec3 whereToLook)
{
        glm::vec3 direction = glm::normalize(cameraPos - whereToLook);
        glm::vec3 Right = glm::normalize(glm::cross(cameraUp, whereToLook));
        cameraUp = glm::normalize(glm::cross(direction, Right));
        cameraFront = -direction;
}

void Camera::render()
{
    auto currentInputHandler = InputHandler::currentInputHandler;

    // if(currentInputHandler->mouseState == GLFW_CURSOR_NORMAL)
    // return;

    const float sensitivity = 0.05f;

    yaw += (currentInputHandler->getXOffset() * sensitivity);
    pitch += (currentInputHandler->getYOffset() * sensitivity);

    if (pitch > 89.0f)
    pitch = 89.0f;
    if (pitch < -89.0f)
    pitch = -89.0f;

    if(currentInputHandler->m_Camera->cameraType == CameraType::TOP_DOWN)
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);
    }
    else if (currentInputHandler->m_Camera->cameraType == CameraType::THIRD_PERSON)
    {
        currentInputHandler->m_Camera->calculateAngleAroundPlayer();
        cameraDistance = calculateHorizontalDistance();
        cameraHeight = calculateVerticalDistance();
        cameraPos = calculateCameraPosition();
        yaw = 180 - (currentInputHandler->m_Camera->playerRot.y  + currentInputHandler->m_Camera->angleAroundPlayer);
    }
}

void Camera::calculateAngleAroundPlayer()
{
    angleAroundPlayer -= yaw;
}

float Camera::calculateHorizontalDistance()
{
    return cameraDistance * cos(glm::radians(pitch));
}

float Camera::calculateVerticalDistance()
{
    return cameraDistance * sin(glm::radians(pitch));
}

glm::vec3 Camera::calculateCameraPosition()
{
    glm::vec3 position;
    float theta = playerRot.y + angleAroundPlayer;
    float offsetX = cameraDistance * sin(glm::radians(theta));
    float offsetZ = cameraDistance * cos(glm::radians(theta));
    position.x = playerPos.x - offsetX;
    position.z = playerPos.z - offsetZ;
    position.y = playerPos.y + cameraHeight;
    return position;
}