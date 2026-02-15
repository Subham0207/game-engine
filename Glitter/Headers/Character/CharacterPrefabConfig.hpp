//
// Created by subha on 04-01-2026.
//

#ifndef GLITTER_CHARACTERPREFABINFO_HPP
#define GLITTER_CHARACTERPREFABINFO_HPP
#include <string>
#include <glm/glm.hpp>

struct CharacterPrefabConfig {
    std::string name;

    std::string classId;

    std::string modelGuid;
    glm::vec3 modelRelativePosition{};
    float capsuleHalfHeight = 0.0f;
    float capsuleRadius = 0.0f;
    glm::vec3 modelScale{1.0f};

    std::string skeletonGuid;
    std::string stateMachineClassId;
    std::string controllerClassId;
};
#endif //GLITTER_CHARACTERPREFABINFO_HPP