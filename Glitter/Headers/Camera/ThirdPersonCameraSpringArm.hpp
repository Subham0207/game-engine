//
// Created by subha on 10-01-2026.
//

#ifndef GLITTER_SPRINGARM_HPP
#define GLITTER_SPRINGARM_HPP
#include <glm/glm.hpp>

#include <Camera/ISpringArm.hpp>
#include <Controls/Input.hpp>


class ThirdPersonCameraSpringArm: ISpringArm
{
public:
    ThirdPersonCameraSpringArm();

    void moveArm(const MouseMoveEvent& e) override;
    glm::vec3 getEndPosition() override;
    void setPivotPos(glm::vec3 pivotPosition) override;
    glm::vec3 getPivotPos() override;
private:
    void processThirdPersonCamera(double xMouseOffsetOnScreen, double yMouseOffsetOnScreen);
    glm::vec3 calculateEndPosition(glm::vec3 playerPos);
    void calculateAngleAroundPlayer(float offset);

    float calculateHorizontalDistance();
    float calculateVerticalDistance();

    glm::vec3 m_springArmEndPosition{};
    glm::vec3 m_springArmPivotPosition{};
    float springArmHorizontalLength;
    float springArmVerticalLength;

    float theta;
    float angleAroundPlayer;
    float springArmLength;

    float yaw;
    float pitch;
};


#endif //GLITTER_SPRINGARM_HPP