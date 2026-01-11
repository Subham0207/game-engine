//
// Created by subha on 10-01-2026.
//

#include <Camera/ThirdPersonCameraSpringArm.hpp>

ThirdPersonCameraSpringArm::ThirdPersonCameraSpringArm()
{
    springArmHorizontalLength = 16.0f;
    springArmVerticalLength = 7.0f;
    angleAroundPlayer = 0.0f;
    theta = 0.0f;
    yaw = -90.0f;
    pitch = 0.0f;
}

void ThirdPersonCameraSpringArm::moveArm()
{
    processThirdPersonCamera(0.0f, 0.0f);
    m_springArmEndPosition = calculateEndPosition(m_springArmPivotPosition);
}

glm::vec3 ThirdPersonCameraSpringArm::getEndPosition()
{
    return m_springArmEndPosition;
}

void ThirdPersonCameraSpringArm::setPivotPos(glm::vec3 pivotPosition)
{
    m_springArmPivotPosition = pivotPosition;
}

glm::vec3 ThirdPersonCameraSpringArm::getPivotPos()
{
    return m_springArmPivotPosition;
}

void ThirdPersonCameraSpringArm::calculateAngleAroundPlayer(float offset)
{
    angleAroundPlayer -= offset * 0.0005;
}

float ThirdPersonCameraSpringArm::calculateHorizontalDistance()
{
    return springArmLength * cos(glm::radians(pitch));
}

float ThirdPersonCameraSpringArm::calculateVerticalDistance()
{
    return springArmLength * sin(glm::radians(pitch));
}

void ThirdPersonCameraSpringArm::processThirdPersonCamera(float xMouseOffsetOnScreen, float yMouseOffsetOnScreen)
{
    calculateAngleAroundPlayer(xMouseOffsetOnScreen);
    pitch -= (yMouseOffsetOnScreen * 0.05); // make it += for inverted vertical input
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -30.0f)
        pitch = -30.0f;
    springArmHorizontalLength = calculateHorizontalDistance();
    springArmVerticalLength = calculateVerticalDistance();
}

glm::vec3 ThirdPersonCameraSpringArm::calculateEndPosition(glm::vec3 playerPos)
{
    glm::vec3 position;
    // Only use angleAroundPlayer to orbit
    theta =  angleAroundPlayer;

    const float offsetX = springArmHorizontalLength * sin(theta);
    const float offsetZ = springArmHorizontalLength * cos(theta);

    position.x = playerPos.x - offsetX;
    position.z = playerPos.z - offsetZ;
    position.y = playerPos.y + springArmVerticalLength;

    return position;
}
