//
// Created by subha on 22-03-2026.
//

#ifndef GLITTER_EDITORSTATEMACHINE_HPP
#define GLITTER_EDITORSTATEMACHINE_HPP

#pragma once
#include <string>

#include "Windowing/StateMachineWindow.hpp"

class EditorStateMachine
{
public:
    EditorStateMachine(std::string characterFilepath, std::string& smFilePath);
    int openEditor();

    template<typename T>
    void load(T& t)
    {
        if (window != nullptr)
            window->load(t);
    }

private:
    std::unique_ptr<StateMachineWindow> window;
};


#endif //GLITTER_EDITORSTATEMACHINE_HPP