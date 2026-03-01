//
// Created by subha on 28-02-2026.
//

#ifndef GLITTER_POPUPSWIDGET_HPP
#define GLITTER_POPUPSWIDGET_HPP
#pragma once
#include "Asset.hpp"

namespace ProjectAsset
{
    class PopupsWidget
    {
        public:
            static void MaterialActionPopup(Asset selectedAsset);
            static void MaterialInstanceActionPopup(Asset selectedAsset);
            static void ModelActionPopup(Asset selectedAsset);
    };
}


#endif //GLITTER_POPUPSWIDGET_HPP