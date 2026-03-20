#pragma once
#include<vector>
#include<iostream>
#include <string>
#include <filesystem>

#include "Asset.hpp"
#include "AssetBrowserPopups.hpp"
namespace fs = std::filesystem;

//Note: These types are of files that can be used in the game engine.
// So FBX is loaded and saved in a custom ModelType format and the game engine deals with these files.
class Level;
namespace ProjectAsset{
    Asset* convertFilenameToAsset(fs::directory_entry entry);


    //Feature: Drag and drop, Able to go through Assets.
    class AssetBrowser{
    public:
        AssetBrowser();
        void RenderAssetBrowser();
        void PopupWidgets();

        std::vector<Asset> assets;

        std::string currentPath;
        bool refreshAssetBrowser = false;

    private:
        bool showAssetBrowser;
        std::string selectedFile;
        int itemsPerRow = 4;
        float padding = 10.0f;
        float itemSize = 64.0f;
        AssetBrowsePopUps openPopup;

        Asset selectedAsset;
        std::string filterFile;

        void LoadAssets();
        void RenderAsset(Asset* asset);
    };
}