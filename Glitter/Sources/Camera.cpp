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
    const glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);

    // Direction FROM camera TO target
    glm::vec3 front = glm::normalize(whereToLook - cameraPos);

    // Build an orthonormal basis
    glm::vec3 right = glm::normalize(glm::cross(front, WORLD_UP));
    glm::vec3 up    = glm::normalize(glm::cross(right, front));

    cameraFront = front;
    cameraUp    = up;
}

void Camera::render()
{
    auto currentInputHandler = InputHandler::currentInputHandler;

    if(currentInputHandler->mouseState == GLFW_CURSOR_NORMAL)
    return;  

    switch (cameraType)
    {
        case CameraType::TOP_DOWN:
            processDefaultCamera(currentInputHandler);
            break;
        case CameraType::THIRD_PERSON:
            processThirdPersonCamera(currentInputHandler);
            break;
        default:
            break;
    }
}

void Camera::processDefaultCamera(InputHandler *currentInputHandler)
{
    const float sensitivity = 0.05f;

    yaw += (currentInputHandler->getXOffset() * sensitivity);
    pitch += (currentInputHandler->getYOffset() * sensitivity);

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
void Camera::processThirdPersonCamera(InputHandler *currentInputHandler)
{
    calculateAngleAroundPlayer(currentInputHandler->getXOffset());
    pitch += (currentInputHandler->getYOffset() * 0.05);
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < 0.0f)
        pitch = 0.0f;
    cameraDistance = calculateHorizontalDistance();
    cameraHeight = calculateVerticalDistance();
    cameraPos = calculateCameraPosition();

    lookAt(playerPos);
}

void Camera::calculateAngleAroundPlayer(float offset)
{
    angleAroundPlayer -= offset * 0.005;
}

float Camera::calculateHorizontalDistance()
{
    return maxDistance * cos(glm::radians(pitch));
}

float Camera::calculateVerticalDistance()
{
    return maxDistance * sin(glm::radians(pitch));
}

glm::vec3 Camera::calculateCameraPosition()
{
    glm::vec3 position;

    // Only use angleAroundPlayer to orbit
    float theta =  playerRot.y + angleAroundPlayer;

    float offsetX = cameraDistance * sin(theta);
    float offsetZ = cameraDistance * cos(theta);

    position.x = playerPos.x - offsetX;
    position.z = playerPos.z - offsetZ;
    position.y = playerPos.y + cameraHeight;

    return position;
}