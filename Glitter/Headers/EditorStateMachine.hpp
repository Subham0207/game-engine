//
// Created by subha on 22-03-2026.
//

#ifndef GLITTER_EDITORSTATEMACHINE_HPP
#define GLITTER_EDITORSTATEMACHINE_HPP

#pragma once
#include <string>

class EditorStateMachine
{
public:
    template<typename T>
    int openEditor(std::string characterFilepath, std::string& smFilePath, T& t);
};


#endif //GLITTER_EDITORSTATEMACHINE_HPP