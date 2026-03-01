//
// Created by subha on 28-02-2026.
//

#ifndef GLITTER_ASSETBROWSERPOPUPWIDGETS_HPP
#define GLITTER_ASSETBROWSERPOPUPWIDGETS_HPP
#pragma once
namespace ProjectAsset
{
    struct AssetBrowsePopUps{
        bool characterPrefab = false;
        bool AI = false;
        bool material = false;
        bool materialInstance = false;
        bool model = false;

        void setAllPopup(bool popupState)
        {
            characterPrefab = popupState;
            AI = popupState;
            material = popupState;
            materialInstance = popupState;
            model = popupState;
        }
    };
}
#endif //GLITTER_ASSETBROWSERPOPUPWIDGETS_HPP