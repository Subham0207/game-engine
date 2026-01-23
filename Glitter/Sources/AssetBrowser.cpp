#pragma once
#include "UI/AssetBrowser.hpp"
#include <imgui.h>
#include <EngineState.hpp>
#include "Helpers/Shared.hpp"
#include <glad/glad.h>
#include <Modals/FileType.hpp>
#include <UI/StatemachineUI.hpp>

#include "Prefab.hpp"
#include "UI/CharacterUI.hpp"

namespace ProjectAsset
{
    AssetBrowser::AssetBrowser()
    {
        this->showAssetBrowser = true;
        currentPath = EngineState::state->currentActiveProjectDirectory;
        LoadAssets();
        selectedAsset = {};
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
                bool openCharacterPopup = false;
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
                                selectedAsset = assets[i];
                                if(selectedAsset.assetType == AssetType::Directory)
                                {
                                    currentPath = fs::path(assets[i].filepath).string();
                                    refreshAssetBrowser = true;
                                }
                                else if(selectedAsset.assetType == AssetType::BlendSpaceType)
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
                                else if(selectedAsset.assetType == AssetType::StateMachineType)
                                {
                                    auto guid = fs::path(selectedAsset.filepath).filename().stem().stem().string();
                                    auto filesMap = getEngineRegistryFilesMap();
                                    if (auto it = filesMap.find(guid); it != filesMap.end())
                                    {
                                        auto statemachine = UI::StatemachineUI::start();
                                        statemachine->load(fs::path(selectedAsset.filepath).parent_path(), guid);
                                    }
                                }
                                else if(selectedAsset.assetType == AssetType::ModelType)
                                {
                                    auto guid = fs::path(selectedAsset.filepath).filename().stem().stem().string();
                                    auto filesMap = getEngineRegistryFilesMap();
                                    if (auto it = filesMap.find(guid); it != filesMap.end())
                                    {
                                        auto model = std::make_shared<Model>();
                                        auto modelPath = fs::path(selectedAsset.filepath).parent_path();
                                        model->load(modelPath, guid);
                                        model->setModelMatrix(glm::identity<glm::mat4>());
                                        auto filename = fs::path(filesMap[guid]).filename().stem().string();
                                        model->setFileName(filename);
                                        getActiveLevel().addRenderable(model);
                                    }
                                    
                                }
                                else if (selectedAsset.assetType == AssetType::CharacterType)
                                {
                                    openCharacterPopup = true;
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
            ImGui::EndChild();

                ImGuiID popup_id = ImGui::GetID("CharacterActionPopup");
                if (openCharacterPopup)
                    ImGui::OpenPopup(popup_id);

                if (ImGui::BeginPopup("CharacterActionPopup"))
                {
                    if (ImGui::MenuItem("Edit in Character UI"))
                    {
                        auto characterPrefab = new CharacterPrefabConfig();
                        Engine::Prefab::readCharacterPrefab(selectedAsset.filepath, *characterPrefab);
                        getUIState().characterUIState->start(*characterPrefab, selectedAsset.filepath);
                    }

                    if (ImGui::MenuItem("Add Character to Level"))
                    {
                        // Your logic to instantiate the character into the current scene
                        // Engine::Level::SpawnCharacter(selectedAsset.filepath);
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Cancel"))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

            ImGui::End();
            }
    }
    
    void AssetBrowser::LoadAssets(){
        assets.clear();
        auto dirIt = fs::directory_iterator(fs::path(currentPath));
        for (const auto& entry : dirIt)
        {
            std::string extension = entry.path().extension().string();
            //We are filterig all filetypes so only meta files go through.
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
            auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
            switch(asset->assetType)
            {
                case AssetType::Directory:
                {
                    auto path = engineFSPath / "EngineAssets/folder.png";
                    asset->textureId = Shared::TextureFromFile(
                        path.u8string().c_str(),
                        "folder.png",
                        false);
                        break;
                }
                case AssetType::CharacterType:
                {
                    auto path = engineFSPath / "EngineAssets/character.png";
                    asset->textureId = Shared::TextureFromFile(
                        path.u8string().c_str(),
                        "character.png",
                        false);
                        break;
                }
                case AssetType::ModelType:
                {
                    auto path = engineFSPath / "EngineAssets/model.png";
                    asset->textureId = Shared::TextureFromFile(
                        path.u8string().c_str(),
                        "model.png",
                        false);
                        break;
                }
                case AssetType::BlendSpaceType:
                {
                    auto path = engineFSPath / "EngineAssets/blendspace.png";
                    asset->textureId = Shared::TextureFromFile(
                        path.u8string().c_str(),
                        "blendspace.png",
                        false);
                        break;
                }
                case AssetType::StateMachineType:
                {
                    auto path = engineFSPath / "EngineAssets/statemachine.png";
                    asset->textureId = Shared::TextureFromFile(
                        path.u8string().c_str(),
                        "statemachine.png",
                        false);
                        break;
                }
                case AssetType::AnimationType:
                {
                    auto path = engineFSPath / "EngineAssets/animation.png";
                    asset->textureId = Shared::TextureFromFile(
                        path.u8string().c_str(),
                        "animation.png",
                        false);
                        break;
                }
                default: 
                {
                    auto path = engineFSPath / "EngineAssets/unknown.png";
                    asset->textureId = Shared::TextureFromFile(
                        path.u8string().c_str(),
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
