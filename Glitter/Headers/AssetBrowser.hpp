#pragma once
#include<vector>
#include<iostream>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

//Note: These types are of files that can be used in the game engine.
// So FBX is loaded and saved in a custom Model format and the game engine deals with these files.

namespace ProjectAsset{
    enum AssetType {
        Directory,
        Model,
        Animation,
        png,
        jpg,
        Unknown
    };

    struct Asset{
        AssetType assetType = AssetType::Unknown;
        std::string filename;
        bool isTextureIdAssigned = false;
        unsigned int textureId;
    };

    Asset* convertFilenameToAsset(fs::directory_entry entry);


    //Feature: Drag and drop, Able to go through Assets.
    class AssetBrowser{
    public:
        AssetBrowser();
        void RenderAssetBrowser();

        std::vector<Asset> assets;

        std::string currentPath;

    private:
        bool showAssetBrowser;
        std::string selectedFile;
        int itemsPerRow = 4;
        float padding = 10.0f;
        float itemSize = 64.0f;

        void LoadAssets();
        void RenderAsset(Asset* asset);
    };
}