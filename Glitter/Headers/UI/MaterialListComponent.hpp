//
// Created by subha on 01-03-2026.
//

#ifndef GLITTER_MATERIALLISTCOMPONENT_HPP
#define GLITTER_MATERIALLISTCOMPONENT_HPP
#pragma once
#include <memory>
#include "Types/MaterialList.hpp"

class Model;
namespace UI
{
    class MaterialListComponent
    {
    public:
        MaterialListComponent();
        void startMaterialsList();
        void drawMaterialsList(Model* selectedModel);
        bool isMaterialListInitialized() const {return materialsListInitialized;}
    private:
        MaterialsList materialsList;
        bool materialsListInitialized = false;
    };
}


#endif //GLITTER_MATERIALLISTCOMPONENT_HPP