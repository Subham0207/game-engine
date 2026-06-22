#include <UI/materialManager.hpp>
#include <imgui.h>
#include "EngineState.hpp"
#include <Helpers/window3d.hpp>
#include <Materials/IMaterial.hpp>

#include "UI/FileExplorer.hpp"
#include "UI/Shared/ComboUI.hpp"
#include "UI/Shared/Utils.hpp"

UI::MaterialManagerUI::MaterialManagerUI()
{
    materialUIModel = MaterialUIModel{};
    operatingOnPath = nullptr;
    showFileExplorerForMaterialEditor = false;
    showMaterialUI = false;
    firstFrame = false;
    showUI =  false;
    materialName.setText("Material");
}

void UI::MaterialManagerUI::drawMaterialEditor()
{
    if (!showMaterialUI)
        return;

    if (ImGui::Begin(materialName.value.c_str(), &showMaterialUI))
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

        Shared::EditableTextUI("MaterialName", materialName);
        ImGui::SeparatorText("Maps");
        DrawField("Albedo", materialUIModel.albedoMapLocation);
        DrawField("Normal", materialUIModel.normalMapLocation);
        DrawField("Metallic", materialUIModel.metallicMapLocation);
        DrawField("Roughness", materialUIModel.roughnessMapLocation);
        DrawField("AO", materialUIModel.aoMapLocation);

        ImGui::SeparatorText("Shaders");
        UI::Shared::comboUI(
            "Vertex",
            materialUIModel.selectedVertexShaderIndex,
            vertexShadersList
        );
        UI::Shared::comboUI(
            "Fragment",
            materialUIModel.selectedFragmentShaderIndex,
            fragmentShadersList
        );

        ImGui::Separator();
        if (ImGui::Button("Compile Material", ImVec2(-FLT_MIN, 0))) {
            // Reload logic
        }

        ImGui::Separator();
        if (ImGui::Button("Save Material", ImVec2(-FLT_MIN, 0))) {
            // Save logic
            //create a Material object and save that...
            const Materials::TextureUnits units;
            units.albedo->name = materialUIModel.albedoMapLocation;
            units.normal->name = materialUIModel.normalMapLocation;
            units.metalness->name = materialUIModel.metallicMapLocation;
            units.roughness->name = materialUIModel.roughnessMapLocation;
            units.ao->name = materialUIModel.aoMapLocation;
            auto vShader = vertexShadersList[Utils::toDataTypeIndex(materialUIModel.selectedVertexShaderIndex)];
            auto fShader = fragmentShadersList[Utils::toDataTypeIndex(materialUIModel.selectedFragmentShaderIndex)];
            auto material = materialRef ?
                (materialRef->Update(
                materialName.value,
                vShader,
                fShader,
                units
                ), std::move(materialRef))
                : std::make_unique<Materials::Material>(
                materialName.value,
                vShader,
                fShader,
                units
                );
            auto dir = EngineState::state->navIntoProjectDir("Assets");
            material->save(dir);

            //cleanup
            // vertexShadersList.clear();
            // fragmentShadersList.clear();
        }
    }
    ImGui::End();

    if (!showFileExplorerForMaterialEditor)
        return;

    if(ImGui::Begin("FileExplorer", &showFileExplorerForMaterialEditor))
    {
        ProjectAsset::RenderFileExplorer(
            getUIState().currentPath,
            EngineState::state->uiState.fileNames,
    EngineState::state->currentActiveProjectDirectory,
    getUIState().selectedFileIndex,
    getUIState().filePath
            );

        if (ImGui::Button("Open"))
        {
            *operatingOnPath = getUIState().filePath;
            //Run some validation based on the selected filepath.
            //Then Load the texture on GPU

            showFileExplorerForMaterialEditor = false;
            operatingOnPath = nullptr;
        }

    }
    ImGui::End();
}

void UI::MaterialManagerUI::startMaterialEditor(std::shared_ptr<Materials::Material> material)
{
    std::vector<fs::path> searchPaths = {EngineState::state->engineInstalledDirectory, EngineState::state->currentActiveProjectDirectory};

    for (const auto& searchPath : searchPaths) {
        if (!fs::exists(searchPath) || !fs::is_directory(searchPath)) continue;

        for (const auto& entry : fs::recursive_directory_iterator(searchPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();

                // Check for .vert or .frag
                if (ext == ".vert") {
                    vertexShadersList.push_back(entry.path().string());
                }
                if (ext == ".frag")
                {
                    fragmentShadersList.push_back(entry.path().string());
                }
            }
        }
    }

    if (material)
    {
        materialRef = material;
        auto findIndex = [](std::vector<std::string>& list, std::string item)
        {
            if (const auto it = std::find(list.begin(),list.end(),item); it != list.end())
            {
                return std::distance(list.begin(), it);
            }
            return -1LL;
        };

        materialName.setText(material->contentName());
        materialUIModel.albedoMapLocation = material->GetTextureUnits().albedo->name;
        materialUIModel.normalMapLocation = material->GetTextureUnits().normal->name;
        materialUIModel.metallicMapLocation = material->GetTextureUnits().metalness->name;
        materialUIModel.roughnessMapLocation = material->GetTextureUnits().roughness->name;
        materialUIModel.aoMapLocation = material->GetTextureUnits().ao->name;

        materialUIModel.selectedVertexShaderIndex = Utils::toUiIndex(findIndex(vertexShadersList, material->getVertexShaderPath()));
        materialUIModel.selectedFragmentShaderIndex = Utils::toUiIndex(findIndex(fragmentShadersList, material->getFragmentShaderPath()));
    }

    showMaterialUI = true;
}

void UI::MaterialManagerUI::setShowUi(bool show)
{
    showUI = show;
}

void UI::MaterialManagerUI::setShowMaterialUI(bool show)
{
    showMaterialUI = show;
}

void UI::MaterialManagerUI::drawMaterialsList()
{
    Model* selectedModel = nullptr;
    auto selectedRenderableIndex = getUIState().selectedRenderableIndex;
    if (selectedRenderableIndex > -1)
    {
        materialListComponent.startMaterialsList();

        auto renderable = getActiveLevel().renderables[selectedRenderableIndex];
        if (auto character = std::dynamic_pointer_cast<Character>(renderable))
        {
            selectedModel = character->model.get();
        }
        if (auto model = std::dynamic_pointer_cast<Model>(renderable))
        {
            selectedModel = model.get();
        }
    }

    materialListComponent.drawMaterialsList(selectedModel);
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
                materialsFoundInModel(character->model.get());
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
        ProjectAsset::RenderFileExplorer(
        getUIState().currentPath,
        EngineState::state->uiState.fileNames,
        EngineState::state->currentActiveProjectDirectory,
        getUIState().selectedFileIndex,
        getUIState().filePath);

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