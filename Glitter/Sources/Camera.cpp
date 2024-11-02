#include "Camera/Camera.hpp"
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