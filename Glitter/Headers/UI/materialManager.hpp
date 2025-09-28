#include <Modals/texture.hpp>
#include <UIState/UIState.hpp>

namespace UI{
    void renderMaterialManagerComponent();

    void materialsFoundInModel();

    void UpdateOrDisplayTexture(
    std::shared_ptr<ProjectModals::Texture> texture,
    int materialIndex, int textureIndex,
    ProjectAsset::FileTypeOperation textureType
    );
}