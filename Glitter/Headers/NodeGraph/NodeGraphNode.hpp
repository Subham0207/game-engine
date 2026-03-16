//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODE_HPP
#define GLITTER_NODE_HPP

#pragma once

#include <string>
#include <utility>

#include <imgui.h>

// Node type used by the NodeGraph editor.
// NOTE: named NodeGraphNode to avoid collisions with any other "Node" types.
class NodeGraphNode
{
public:
    NodeGraphNode() = default;
    NodeGraphNode(int id, std::string name, float x = 0.0f, float y = 0.0f)
        : m_id(id), m_name(std::move(name)), m_x(x), m_y(y)
    {
    }

    [[nodiscard]] int id() const { return m_id; }
    [[nodiscard]] const std::string& name() const { return m_name; }

    [[nodiscard]] float x() const { return m_x; }
    [[nodiscard]] float y() const { return m_y; }
    void setXY(float x, float y)
    {
        m_x = x;
        m_y = y;
    }

    [[nodiscard]] bool positionSet() const { return m_positionSet; }
    void markPositionSet(bool set = true) { m_positionSet = set; }

    // When a node is created via context menu, we store its desired spawn position
    // in screen space and apply it once on first draw.
    void setSpawnPosScreen(const ImVec2& p)
    {
        m_spawnPosScreen = p;
        m_hasSpawnPos = true;
    }
    [[nodiscard]] bool hasSpawnPosScreen() const { return m_hasSpawnPos; }
    [[nodiscard]] ImVec2 spawnPosScreen() const { return m_spawnPosScreen; }

private:
    int m_id = -1;
    std::string m_name;
    float m_x = 0.0f;
    float m_y = 0.0f;
    bool m_positionSet = false;

    bool m_hasSpawnPos = false;
    ImVec2 m_spawnPosScreen{0.0f, 0.0f};
};


#endif //GLITTER_NODE_HPP