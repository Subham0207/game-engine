#pragma once
#include "UI/AssetBrowser.hpp"
#include <imgui.h>
#include <EngineState.hpp>
#include "Helpers/Shared.hpp"
#include <glad/glad.h>
#include <Modals/FileType.hpp>
#include <UI/StatemachineUI.hpp>

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
                                auto selectedAsset = assets[i];
                                if(selectedAsset.assetType == AssetType::Directory)
                                {
                                    currentPath = fs::path(assets[i].filepath).string();
                                    refreshAssetBrowser = true;
                                }

                                if(selectedAsset.assetType == AssetType::BlendSpaceType)
                                {
                                    auto guid = fs::path(selectedAsset.filepath).filename().stem().stem().string();
                                    auto filesMap = getEngineRegistryFilesMap();
                                    if (auto it = filesMap.find(guid); it != filesMap.end())
                                    {
                                        auto blendspace = new BlendSpace2D();
                                        blendspace->load(fs::path(selectedAsset.filepath).parent_path(), guid);
                                        getUIState().blendspace2DUIState->UIOpenedForBlendspace = blendspace;
                                        getUIState().blendspace2DUIState->showBlendspaceUI = true;
                                    }
                                }

                                if(selectedAsset.assetType == AssetType::StateMachineType)
                                {
                                    auto guid = fs::path(selectedAsset.filepath).filename().stem().stem().string();
                                    auto filesMap = getEngineRegistryFilesMap();
                                    if (auto it = filesMap.find(guid); it != filesMap.end())
                                    {
                                        auto statemachine = new Controls::StateMachine();
                                        statemachine->load(fs::path(selectedAsset.filepath).parent_path(), guid);
                                        getUIState().statemachineUIState->UIOpenedForStatemachine = statemachine;
                                        getUIState().statemachineUIState->showStateMachineUI = true;
                                        getUIState().statemachineUIState->firstFrame = true;
                                    }
                                }

                                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                                    ImGui::SetDragDropPayload("ASSET_PATH", selectedAsset.filepath.c_str(),  selectedAsset.filepath.size() + 1);
                                    ImGui::Text("Dragging %s", selectedAsset.filepath.c_str());
                                    ImGui::EndDragDropSource();
                                }
                            }
                            ImGui::TextWrapped("%s", assets[i].filename.c_str());
                        }
                    }
                    ImGui::EndTable();

                }
            }
            ImGui::EndChild();
            ImGui::End();
    }
    
    void AssetBrowser::LoadAssets(){
        assets.clear();
        auto dirIt = fs::directory_iterator(fs::path(currentPath));
        for (const auto& entry : dirIt)
        {
            std::string extension = entry.path().extension().string();
            if (!extension.empty() && extension[0] == '.') extension.erase(0, 1);
            if(
                !(extension == toString(FileType::CharacterType) ||
                extension == toString(FileType::ModelType)||
                extension == toString(FileType::BlendSpaceType)||
                extension == toString(FileType::StateMachineType)||
                extension == toString(FileType::AnimationType))
            )
            {
                auto asset = convertFilenameToAsset(entry, extension);
                assets.push_back(*asset);
            }
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
                case AssetType::CharacterType:
                {
                    asset->textureId = Shared::TextureFromFile(
                        "./EngineAssets/character.png",
                        "character.png",
                        false);
                        break;
                }
                case AssetType::ModelType:
                {
                    asset->textureId = Shared::TextureFromFile(
                        "./EngineAssets/model.png",
                        "model.png",
                        false);
                        break;
                }
                case AssetType::BlendSpaceType:
                {
                    asset->textureId = Shared::TextureFromFile(
                        "./EngineAssets/blendspace.png",
                        "blendspace.png",
                        false);
                        break;
                }
                case AssetType::StateMachineType:
                {
                    asset->textureId = Shared::TextureFromFile(
                        "./EngineAssets/statemachine.png",
                        "statemachine.png",
                        false);
                        break;
                }
                case AssetType::AnimationType:
                {
                    asset->textureId = Shared::TextureFromFile(
                        "./EngineAssets/animation.png",
                        "animation.png",
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

    Asset* convertFilenameToAsset(std::filesystem::directory_entry entry, std::string extension)
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

        if (
            entry.is_regular_file() && entry.path().extension() == ".json" &&
            entry.path().filename().string().find(".meta") != std::string::npos
        )
        {
            auto justMetaGuid = entry.path().stem().stem().string();
            auto filepath = fs::path(getEngineRegistryFilesMap()[justMetaGuid]).filename().string();
            asset->filename = filepath;
            if(Shared::endsWith(filepath, std::string(toString(FileType::CharacterType))))
            asset->assetType = AssetType::CharacterType;
            if(Shared::endsWith(filepath, std::string(toString(FileType::ModelType))))
            asset->assetType = AssetType::ModelType;
            if(Shared::endsWith(filepath, std::string(toString(FileType::BlendSpaceType))))
            asset->assetType = AssetType::BlendSpaceType;
            if(Shared::endsWith(filepath, std::string(toString(FileType::StateMachineType))))
            asset->assetType = AssetType::StateMachineType;
            if(Shared::endsWith(filepath, std::string(toString(FileType::AnimationType))))
            asset->assetType = AssetType::AnimationType;
        }

        return asset;
    }
}