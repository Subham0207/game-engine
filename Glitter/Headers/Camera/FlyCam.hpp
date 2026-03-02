//
// Created by subha on 17-01-2026.
//

#ifndef GLITTER_FLYCAM_HPP
#define GLITTER_FLYCAM_HPP

#include <Camera/Camera.hpp>

class FlyCam: public Camera
{
public:
    FlyCam(const std::string& name);
    void onMouseMove(const MouseMoveEvent& e) override;
    void moveCamera(double xOffset, double yOffset);
    float& getSensitivityRef(){return sensitivity;}
    float& getCameraSpeedRef(){return cameraSpeed;}
private:
    double yaw;
    double pitch;

    float sensitivity;
    float cameraSpeed;
};


#endif //GLITTER_FLYCAM_HPP