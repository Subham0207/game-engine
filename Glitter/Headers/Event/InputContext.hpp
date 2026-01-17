//
// Created by subha on 17-01-2026.
//

#ifndef GLITTER_INPUTCONTEXT_HPP
#define GLITTER_INPUTCONTEXT_HPP


#pragma once
#include <Event/EventQueue.hpp>

struct InputContext {
    EventQueue* queue = nullptr;

    bool hasLast = false;
    double lastX = 0.0;
    double lastY = 0.0;
};



#endif //GLITTER_INPUTCONTEXT_HPP