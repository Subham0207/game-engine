//
// Created by subha on 14-02-2026.
//

#ifndef GLITTER_AIPREFAB_HPP
#define GLITTER_AIPREFAB_HPP
#include <string>

/*
 * AiPrefab configuration will be used to spawn an AI object of classId.
 * characterPrefabAssetId is used to spawn the characterObject.
 * Then we can get the playerController used in characterObject and send to AI object
 */
class AiPrefab
{
public:
    std::string name;

    std::string classId;

    std::string characterPrefabAssetId;
};


#endif //GLITTER_AIPREFAB_HPP