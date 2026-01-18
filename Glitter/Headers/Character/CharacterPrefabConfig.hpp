//
// Created by subha on 04-01-2026.
//

#ifndef GLITTER_CHARACTERPREFABINFO_HPP
#define GLITTER_CHARACTERPREFABINFO_HPP
#include <string>

struct CharacterPrefabConfig {
    std::string name;

    std::string classId;
    std::string modelGuid;
    std::string skeletonGuid;
    std::string stateMachineClassId;

};
#endif //GLITTER_CHARACTERPREFABINFO_HPP