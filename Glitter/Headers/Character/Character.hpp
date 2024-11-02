#pragma once
#include "3DModel/model.hpp"
#include "3DModel/Animation/Animator.hpp"
#include "Camera/Camera.hpp"
#include "glm/glm.hpp"

class Character
{
public:
    Character(){
        animator = new Animator();
    };

    Model* model;
    Animator* animator;

private:
    Camera* camera;

    glm::mat4 transformation;
};