//
// Created by subha on 17-01-2026.
//

#ifndef GLITTER_EVENT_HPP
#define GLITTER_EVENT_HPP

#pragma once
#include <cstdint>
#include <string>
#include <utility>

#include "GLFW/glfw3.h"

enum class EventType : uint32_t {
    MouseMove,
    MouseButton,
    Key,
    WindowResize,
    HUDUpdate,
    ActivateHUD
};

enum class HUDUpdateOperation : uint32_t
{
    SetStyle,
    SetText
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

struct HUDUpdateEvent final : Event {
    HUDUpdateOperation operation = HUDUpdateOperation::SetStyle;
    std::string elementId;
    std::string property;
    std::string value;

    HUDUpdateEvent(HUDUpdateOperation operation, std::string elementId, std::string property, std::string value)
        : operation(operation),
          elementId(std::move(elementId)),
          property(std::move(property)),
          value(std::move(value))
    {}

    EventType type() const override { return EventType::HUDUpdate; }
};

struct ActivateHUDEvent final : Event {
    std::string hudKey;

    explicit ActivateHUDEvent(std::string hudKey)
        : hudKey(std::move(hudKey))
    {}

    EventType type() const override { return EventType::ActivateHUD; }
};

#endif //GLITTER_EVENT_HPP