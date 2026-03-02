#pragma once
#include <Modals/texture.hpp>
#include <UIState/UIState.hpp>

#include "MaterialListComponent.hpp"
#include "Shared/EditableText.hpp"

namespace UI{
    struct MaterialUIModel
    {
        std::string albedoMapLocation;
        std::string normalMapLocation;
        std::string metallicMapLocation;
        std::string roughnessMapLocation;
        std::string aoMapLocation;

        int selectedVertexShaderIndex;
        int selectedFragmentShaderIndex;
    };
    class MaterialManagerUI
    {
    public:
        MaterialManagerUI();

        void drawMaterialEditor();
        void startMaterialEditor(std::shared_ptr<Materials::Material> material);
        void setShowUi(bool show);
        void setShowMaterialUI(bool show);

        void drawMaterialsList();

    private:

        //MaterialsLists
        MaterialListComponent materialListComponent;

        //MaterialEditorUI
        std::string* operatingOnPath;
        bool showFileExplorerForMaterialEditor;
        std::shared_ptr<Materials::Material> materialRef;

        MaterialUIModel materialUIModel;
        std::vector<std::string> vertexShadersList;
        std::vector<std::string> fragmentShadersList;

        Shared::EditableText materialName;
        bool showMaterialUI;
        bool firstFrame;

        //Manage Materials attached to a model
        bool showUI;
        void ManageMaterialsOfModel();
        void textureUnitsEditor(Materials::TextureUnits& textureUnits, int materialIndex);
        void materialsFoundInModel(Model* model);
        void UpdateOrDisplayTexture(
        std::shared_ptr<ProjectModals::Texture> texture,
        int materialIndex, int textureIndex
        );

    };
}
