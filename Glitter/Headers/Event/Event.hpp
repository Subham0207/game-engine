//
// Created by subha on 17-01-2026.
//

#ifndef GLITTER_EVENT_HPP
#define GLITTER_EVENT_HPP

#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <utility>

#include <glm/glm.hpp>

#include "GLFW/glfw3.h"

enum class EventType : uint32_t {
    MouseMove,
    MouseButton,
    Key,
    WindowResize,
    HUDUpdate,
    ActivateHUD,
    BodiesCollided,
    SensorsEntered,
    SensorsExited
};

class Renderable;

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

struct BodiesCollidedEvent final : Event {
    Renderable* bodyA = nullptr;
    Renderable* bodyB = nullptr;
    std::string bodyAInstanceId;
    std::string bodyBInstanceId;
    std::vector<glm::vec3> contactPoints;

    BodiesCollidedEvent(
        Renderable* bodyA,
        Renderable* bodyB,
        std::string bodyAInstanceId,
        std::string bodyBInstanceId,
        std::vector<glm::vec3> contactPoints)
        : bodyA(bodyA),
          bodyB(bodyB),
          bodyAInstanceId(std::move(bodyAInstanceId)),
          bodyBInstanceId(std::move(bodyBInstanceId)),
          contactPoints(std::move(contactPoints))
    {}

    EventType type() const override { return EventType::BodiesCollided; }
};

struct SensorsEnteredEvent final : Event {
    Renderable* sensorOwner = nullptr;
    Renderable* otherBody = nullptr;
    std::string sensorOwnerInstanceId;
    std::string otherBodyInstanceId;

    SensorsEnteredEvent(
        Renderable* sensorOwner,
        Renderable* otherBody,
        std::string sensorOwnerInstanceId,
        std::string otherBodyInstanceId)
        : sensorOwner(sensorOwner),
          otherBody(otherBody),
          sensorOwnerInstanceId(std::move(sensorOwnerInstanceId)),
          otherBodyInstanceId(std::move(otherBodyInstanceId))
    {}

    EventType type() const override { return EventType::SensorsEntered; }
};

struct SensorsExitedEvent final : Event {
    Renderable* sensorOwner = nullptr;
    Renderable* otherBody = nullptr;
    std::string sensorOwnerInstanceId;
    std::string otherBodyInstanceId;

    SensorsExitedEvent(
        Renderable* sensorOwner,
        Renderable* otherBody,
        std::string sensorOwnerInstanceId,
        std::string otherBodyInstanceId)
        : sensorOwner(sensorOwner),
          otherBody(otherBody),
          sensorOwnerInstanceId(std::move(sensorOwnerInstanceId)),
          otherBodyInstanceId(std::move(otherBodyInstanceId))
    {}

    EventType type() const override { return EventType::SensorsExited; }
};

#endif //GLITTER_EVENT_HPP