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

    virtual void moveArm() = 0;
    virtual glm::vec3 getEndPosition() = 0;
    virtual void setPivotPos(glm::vec3 pivotPosition) = 0;
    virtual glm::vec3 getPivotPos() = 0;
};
#endif //GLITTER_ISPRINGARM_HPP