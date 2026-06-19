//
// Created by subha on 01-03-2026.
//

#ifndef GLITTER_MODELUI_HPP
#define GLITTER_MODELUI_HPP
#pragma once
#include <string>
#include <vector>

#include "UI/MaterialListComponent.hpp"
#include "UI/Shared/EditableText.hpp"

namespace UI
{
    class ModelUI
    {
    public:
        ModelUI();
        ~ModelUI();

        void start(const std::shared_ptr<Model>& model);
        void draw();
    private:
        bool showUI;

        std::shared_ptr<Model> selectedModel;
        std::vector<std::string> registeredModelClassNames;
        int selectedModelClassIndex = 0;
        Shared::EditableText ModelName;
        MaterialListComponent materialListComponent;
        std::string customColliderAssetPath;
        std::string colliderValidationMessage;
    };
}


#endif //GLITTER_MODELUI_HPP