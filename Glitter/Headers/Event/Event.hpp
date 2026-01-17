//
// Created by subha on 17-01-2026.
//

#ifndef GLITTER_EVENT_HPP
#define GLITTER_EVENT_HPP

#pragma once
#include <cstdint>

#include "GLFW/glfw3.h"

enum class EventType : uint32_t {
    MouseMove,
    MouseButton,
    Key,
    WindowResize
};

struct Event {
    virtual ~Event() = default;
    virtual EventType type() const = 0;
    bool handled = false; // optional
};

struct MouseMoveEvent final : Event {
    double xOffset = 0.0;
    double yOffset = 0.0;
    unsigned int mouseState = GLFW_CURSOR_DISABLED;

    MouseMoveEvent(double dx, double dy, unsigned int mouseState)
        : xOffset(dx), yOffset(dy), mouseState(mouseState){}

    EventType type() const override { return EventType::MouseMove; }
};

#endif //GLITTER_EVENT_HPP