//
// Created by subha on 28-02-2026.
//

#ifndef GLITTER_ASSET_HPP
#define GLITTER_ASSET_HPP
#pragma once
#include "AssetType.hpp"
#include <string>

namespace ProjectAsset
{
    struct Asset{
        AssetType assetType = AssetType::Unknown;
        std::string filepath;
        std::string filename;
        bool isTextureIdAssigned = false;
        unsigned int textureId;
    };
}
#endif //GLITTER_ASSET_HPP