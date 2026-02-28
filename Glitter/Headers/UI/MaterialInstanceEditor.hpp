//
// Created by subha on 28-02-2026.
//

#ifndef GLITTER_MATERIALINSTANCEEDITOR_HPP
#define GLITTER_MATERIALINSTANCEEDITOR_HPP
#include <memory>

#include "Materials/MaterialInstance.hpp"
#include "Shared/EditableText.hpp"
#include "Types/MaterialList.hpp"

namespace UI
{
    struct MaterialInstanceUIModal
    {
        std::string albedoMapLocation;
        std::string normalMapLocation;
        std::string metallicMapLocation;
        std::string roughnessMapLocation;
        std::string aoMapLocation;

        int parentMaterialIndex;
    };
    class MaterialInstanceEditor
    {
    public:
        MaterialInstanceEditor();
        void setShowUI(bool show);
        void start(std::shared_ptr<Materials::MaterialInstance> materialInstance);
        void drawUI();

    private:
        bool showUI;
        std::shared_ptr<Materials::MaterialInstance> materialInstanceRef;
        Shared::EditableText materialInstanceName;
        MaterialsList materialsList;
        bool materialsListInitialized = false;
        MaterialInstanceUIModal materialInstanceUIModal;

        //Vars for managing opening/assigning filepaths to texture maps;
        std::string* operatingOnPath;
        bool openFileExplorer;
        //------------
    };
}


#endif //GLITTER_MATERIALINSTANCEEDITOR_HPP