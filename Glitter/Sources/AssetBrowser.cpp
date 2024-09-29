#include <AssetBrowser.hpp>
#include <imgui.h>
#include <state.hpp>

namespace ProjectAsset
{
    AssetBrowser::AssetBrowser()
    {
        this->showAssetBrowser = true;
    }

    void AssetBrowser::RenderAssetBrowser(){
            if (ImGui::Begin("Asset Browser", &showAssetBrowser))
            {
                static std::string currentPath = fs::current_path().string();
                for (const auto& entry : fs::directory_iterator(currentPath))
                {
                    auto asset = convertFilenameToAsset(entry);
                    assets.push_back(*asset);
                }
                
                if(ImGui::Button("Go to Root of project"))
                {
                currentPath = State::state->projectRootLocation;
                }

                // Up button to go to the parent directory
                if (ImGui::Button("Up"))
                {
                    currentPath = fs::path(currentPath).parent_path().string();
                }

                // List files and directories
                if (ImGui::BeginListBox("##Assets", ImVec2(-FLT_MIN, 100)))
                {
                    for (const auto& asset : assets)
                    {
                        const bool isSelected = (selectedFile == asset.filename);
                        if (ImGui::Selectable(asset.filename.c_str(), isSelected))
                        {
                            selectedFile = asset.filename;

                            // If a directory is selected, navigate into it
                            if (fs::is_directory(asset.filename))
                            {
                                currentPath = asset.filename;
                            }
                        }
                        
                        // ImVec2 size = ImVec2(32.0f, 32.0f);                         // Size of the image we want to make visible
                        // ImVec2 uv0 = ImVec2(0.0f, 0.0f);                            // UV coordinates for lower-left
                        // ImVec2 uv1 = ImVec2(32.0f / my_tex_w, 32.0f / my_tex_h);    // UV coordinates for (32,32) in our texture
                        // ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);             // Black background
                        // ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);           // No tint
                        // if (ImGui::ImageButton("", my_tex_id, size, uv0, uv1, bg_col, tint_col))
                        // {
                            
                        // }

                    }
                    ImGui::EndListBox();
                }

                ImGui::End();
            }
    }

    Asset* convertFilenameToAsset(std::filesystem::directory_entry entry)
    {
        auto asset = new Asset();
        if(entry.is_directory())
        {
            asset->assetType = AssetType::Directory;
            asset->filename = entry.path().string();
            return asset;
        }

        asset->filename = entry.path().stem().string();
        auto extension = entry.path().extension().string();
        if(extension == "model")
        {
            asset->assetType = AssetType::Model;
        }
        if(extension == "anim")
        {
            asset->assetType = AssetType::Animation;
        }

        return asset;
    }
}