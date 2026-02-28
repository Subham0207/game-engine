//
// Created by subha on 28-02-2026.
//

#ifndef GLITTER_MATERIALLIST_HPP
#define GLITTER_MATERIALLIST_HPP
#pragma once
#include <vector>
#include <string>

namespace UI
{
    struct MaterialsList
    {
        std::vector<std::string> materialGuids;
        std::vector<std::string> materialNames;

        void clear()
        {
            materialGuids.clear();
            materialNames.clear();
        }
    };
}
#endif //GLITTER_MATERIALLIST_HPP