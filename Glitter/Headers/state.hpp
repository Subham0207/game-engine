#pragma once
#include <glm/glm.hpp>

class State{
public:
    State(){}
    static State* state;
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;

    glm::vec3 rayEnd;
};