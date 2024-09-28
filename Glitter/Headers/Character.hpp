#pragma once
#include <model.hpp>
#include <Animator.hpp>
#include <Camera.hpp>
#include <glm/glm.hpp>                                                                           

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