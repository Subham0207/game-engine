//
// Created by subha on 01-03-2026.
//

#ifndef GLITTER_MODELUI_HPP
#define GLITTER_MODELUI_HPP
#pragma once
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
        Shared::EditableText ModelName;
        MaterialListComponent materialListComponent;
    };
}


#endif //GLITTER_MODELUI_HPP