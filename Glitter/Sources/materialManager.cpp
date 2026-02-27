#include <UI/materialManager.hpp>
#include <imgui.h>
#include "EngineState.hpp"
#include <Helpers/window3d.hpp>
#include <Materials/IMaterial.hpp>

#include "UI/FileExplorer.hpp"

UI::MaterialManagerUI::MaterialManagerUI()
{
    materialUIModel = MaterialUIModel{};
    operatingOnPath = nullptr;
    showFileExplorerForMaterialEditor = false;
    showMaterialUI = false;
    firstFrame = false;
    showUI =  false;
}

void UI::MaterialManagerUI::draw()
{
    if (!showMaterialUI)
        return;

    if (ImGui::Begin("Material's UI", &showMaterialUI))
    {
        auto DrawField = [this](const char* label, std::string& path) {
            ImGui::Text("%s:", label);

            // Show the path or a placeholder if empty
            if (path.empty()) {
                ImGui::TextDisabled("  (No file selected)");
            } else {
                // Using Bullet to indent the path slightly for visual hierarchy
                ImGui::BulletText("%s", path.c_str());
            }

            ImGui::SameLine(ImGui::GetWindowWidth() - 70); // Align button to the right
            std::string btnLabel = "Browse##" + std::string(label);

            if (ImGui::Button(btnLabel.c_str())) {
                operatingOnPath = &path;
                showFileExplorerForMaterialEditor = true;
            }
            ImGui::Spacing();
        };

        ImGui::SeparatorText("Maps");
        DrawField("Albedo", materialUIModel.albedoMapLocation);
        DrawField("Normal", materialUIModel.normalMapLocation);
        DrawField("Metallic", materialUIModel.metallicMapLocation);
        DrawField("Roughness", materialUIModel.roughnessMapLocation);
        DrawField("AO", materialUIModel.aoMapLocation);

        ImGui::SeparatorText("Shaders");
        DrawField("Vertex", materialUIModel.vertexShaderLocation);
        DrawField("Fragment", materialUIModel.fragmentShaderLocation);

        ImGui::Separator();
        if (ImGui::Button("Compile Material", ImVec2(-FLT_MIN, 0))) {
            // Reload logic
        }
    }
    ImGui::End();

    if (!showFileExplorerForMaterialEditor)
        return;

    if(ImGui::Begin("FileExplorer", &showFileExplorerForMaterialEditor))
    {
        ProjectAsset::RenderFileExplorer(getUIState().currentPath, EngineState::state->uiState.fileNames);

        if (ImGui::Button("Open"))
        {
            *operatingOnPath = getUIState().currentPath;
            //Run some validation based on the selected filepath.
            //Then Load the texture on GPU

            showFileExplorerForMaterialEditor = false;
            operatingOnPath = nullptr;
        }

    }
    ImGui::End();
}

void UI::MaterialManagerUI::setShowUi(bool show)
{
    showUI = show;
}

void UI::MaterialManagerUI::setShowMaterialUI(bool show)
{
    showMaterialUI = show;
}

void UI::MaterialManagerUI::ManageMaterialsOfModel()
{
    if (ImGui::Begin("Material Manager", &showUI))
    {
        if (getUIState().selectedRenderableIndex > -1)
        {
            auto renderable = getActiveLevel().renderables[getUIState().selectedRenderableIndex];
            if (auto character = std::dynamic_pointer_cast<Character>(renderable))
            {
                materialsFoundInModel(character->model);
            }
            if (auto model = std::dynamic_pointer_cast<Model>(renderable))
            {
                materialsFoundInModel(model.get());
            }
        }
    }
    ImGui::End();

}

void UI::MaterialManagerUI::textureUnitsEditor(Materials::TextureUnits& textureUnits, int materialIndex)
{
    int textureIndex = 2; // This is just for UI purposes

    ImGui::Text("Albedo");
    UpdateOrDisplayTexture(textureUnits.albedo,
    materialIndex,textureIndex);

    ImGui::Text("Normal");
    ImGui::SameLine();
    UpdateOrDisplayTexture( textureUnits.normal,
    materialIndex,textureIndex+1);

    ImGui::Text("Metalness");
    ImGui::SameLine();
    UpdateOrDisplayTexture( textureUnits.metalness,
    materialIndex,textureIndex+2);

    ImGui::Text("Roughness");
    ImGui::SameLine();
    UpdateOrDisplayTexture( textureUnits.roughness,
    materialIndex,textureIndex+3);

    ImGui::Text("AO");
    ImGui::SameLine();
    UpdateOrDisplayTexture( textureUnits.ao,
    materialIndex,textureIndex+4);
}
void UI::MaterialManagerUI::materialsFoundInModel(Model* model)
{
    if (model)
    {
        for (int i = 0;i < model->meshes.size(); i++)
        {
            if (auto material = model->meshes[i].mMaterial)
            {
                auto& textureUnits = material->GetTextureUnits();
                textureUnitsEditor(textureUnits,i);
            }
        }
    }
}

void UI::MaterialManagerUI::UpdateOrDisplayTexture(
    std::shared_ptr<ProjectModals::Texture> texture,
    int materialIndex, int textureIndex
)
{
    static std::shared_ptr<ProjectModals::Texture> currentTexture = nullptr;
    static bool showFileExplorer = false;

    ImGui::SameLine();
    if(texture)
    ImGui::Text("%s", texture->name.c_str());
    ImGui::SameLine();
    if(ImGui::Button(("MaterialBrowse##" + std::to_string(textureIndex) + std::to_string(materialIndex)).c_str()))
    {
        currentTexture = texture;
        showFileExplorer = true;
    }

    // Now open fileExplorer and assign texture.name to whatever is selected and opened.
    if(ImGui::Begin("FileExplorer", &showFileExplorer))
    {
        ProjectAsset::RenderFileExplorer(getUIState().currentPath, EngineState::state->uiState.fileNames);

        if (ImGui::Button("Open"))
        {
            currentTexture->name = getUIState().currentPath;
            //Then Load the texture on GPU

            showFileExplorer = false;
            currentTexture = nullptr;
        }

    }
    ImGui::End();
}