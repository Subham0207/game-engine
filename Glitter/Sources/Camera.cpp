#include "Camera/Camera.hpp"
#include <glad/glad.h>
#include <Controls/Input.hpp>
#include <Modals/CameraType.hpp>
#include <Helpers/glitter.hpp>

Camera::Camera()
{
}

Camera::Camera(std::string name): cameraName(name){
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

glm::vec3 Camera::getRight()
{
    return cameraRight;
}

void Camera::setupView()
{
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void Camera::setupProjection()
{
    float aspect = (float)mWidth / (float)mHeight;
    projection = glm::perspective(glm::radians(fov),  aspect, 0.001f, 1000.0f);
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
    cameraRight = glm::normalize(glm::cross(front, WORLD_UP));
    glm::vec3 up = glm::normalize(glm::cross(cameraRight, front));

    cameraFront = front;
    cameraUp    = up;
}

void Camera::onMouseMove(const MouseMoveEvent& e)
{
    if(e.mouseState == GLFW_CURSOR_NORMAL)
        return;
}

void Camera::tick(glm::vec3 playerPos, glm::vec3 playerRot)
{
    //most likely we don't need this since we will call this in Character derived class itself after assigning position of camera.
    lookAt(playerPos);
}