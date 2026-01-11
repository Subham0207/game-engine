//
// Created by subha on 11-01-2026.
//

#ifndef GLITTER_ISPRINGARM_HPP
#define GLITTER_ISPRINGARM_HPP

#include <glm/glm.hpp>

class ISpringArm
{
public:
    virtual ~ISpringArm() = default;

    virtual void moveArm();
    virtual glm::vec3 getEndPosition();
    virtual void setPivotPos(glm::vec3 pivotPosition);
    virtual glm::vec3 getPivotPos();
};
#endif //GLITTER_ISPRINGARM_HPP