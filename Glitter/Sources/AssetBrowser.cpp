#pragma once
#include "UI/AssetBrowser.hpp"
#include <imgui.h>
#include <EngineState.hpp>
#include "Helpers/Shared.hpp"
#include <glad/glad.h>

namespace ProjectAsset
{
    AssetBrowser::AssetBrowser()
    {
        this->showAssetBrowser = true;
        currentPath = fs::current_path().string();
        LoadAssets();
    }

    void AssetBrowser::RenderAssetBrowser(){
            if (ImGui::Begin("Asset Browser", &showAssetBrowser))
            {
                if(refreshAssetBrowser)
                {
                    LoadAssets();
                    refreshAssetBrowser = false;
                }

                if(ImGui::Button("Go to Root of project"))
                {
                    currentPath = EngineState::state->currentActiveProjectDirectory;
                    refreshAssetBrowser = true;
                }

                // Up button to go to the parent directory
                if (ImGui::Button("Up"))
                {
                    currentPath = fs::path(currentPath).parent_path().string();
                    refreshAssetBrowser = true;
                }


                ImVec2 availableSize = ImGui::GetContentRegionAvail();
                int itemsPerRow = std::max(1, (int)((availableSize.x + padding) / (itemSize + padding)));
                // List files and directories
                if (ImGui::BeginChild("AssetGridScrollable", ImVec2(availableSize.x, availableSize.y), false, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    if (ImGui::BeginTable("AssetGridDynamic", itemsPerRow, ImGuiTableFlags_None))
                    {
                        for (int i = 0; i< assets.size();i++)
                        {
                            ImGui::TableNextColumn();

                            RenderAsset(&assets[i]);
                            if(ImGui::ImageButton(
                                ("##image" + std::to_string(i)).c_str(),   // Unique label for ImGui ID system
                                (ImTextureID)(intptr_t)assets[i].textureId,
                                ImVec2(itemSize, itemSize),
                                ImVec2(0, 0),
                                ImVec2(1, 1),
                                ImVec4(0, 0, 0, 0),
                                ImVec4(1, 1, 1, 1)
                            ))
                            {
                                if(assets[i].assetType == AssetType::Directory)
                                {
                                    std::cout << "Directory clicked" << std::endl;
                                    currentPath = fs::path(assets[i].filepath).string();
                                    refreshAssetBrowser = true;
                                }

                                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                                    ImGui::SetDragDropPayload("ASSET_PATH", assets[i].filepath.c_str(),  assets[i].filepath.size() + 1);
                                    ImGui::Text("Dragging %s", assets[i].filepath.c_str());
                                    ImGui::EndDragDropSource();
                                }
                            }
                            ImGui::TextWrapped("%s", assets[i].filename.c_str());
                        }
                        ImGui::EndTable();
                    }

                    ImGui::EndChild();
                }

                ImGui::End();
            }
    }
    
    void AssetBrowser::LoadAssets(){
        assets.clear();
        for (const auto& entry : fs::directory_iterator(currentPath))
        {
            auto asset = convertFilenameToAsset(entry);
            assets.push_back(*asset);
        }
    }

    void AssetBrowser::RenderAsset(Asset* asset)
    {
        if(!asset->isTextureIdAssigned)
        {
            switch(asset->assetType)
            {
                case AssetType::Directory:
                {
                    asset->textureId = Shared::TextureFromFile(
                        "./EngineAssets/folder.png",
                        "folder.png",
                        false);
                        break;
                }
                default: 
                {
                    asset->textureId = Shared::TextureFromFile(
                        "./EngineAssets/unknown.png",
                        "unknown.png",
                        false);
                }

            }

            asset->isTextureIdAssigned = true;
        }
    }

    Asset* convertFilenameToAsset(std::filesystem::directory_entry entry)
    {
        auto asset = new Asset();
        if(entry.is_directory())
        {
            asset->assetType = AssetType::Directory;
            asset->filepath = entry.path().string();
            asset->filename = entry.path().filename().string();
            return asset;
        }

        // asset->filename = entry.path().stem().string();
        asset->filepath = entry.path().string();
        asset->filename = entry.path().filename().string();
        auto extension = entry.path().extension().string();
        if(extension == "model")
        {
            asset->assetType = AssetType::ModelType;
        }
        if(extension == "anim")
        {
            asset->assetType = AssetType::AnimationType;
        }

        return asset;
    }
}