//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODE_HPP
#define GLITTER_NODE_HPP

#pragma once

#include <string>
#include <utility>

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

    int id() const { return m_id; }
    const std::string& name() const { return m_name; }

    float x() const { return m_x; }
    float y() const { return m_y; }
    void setXY(float x, float y)
    {
        m_x = x;
        m_y = y;
    }

    bool positionSet() const { return m_positionSet; }
    void markPositionSet(bool set = true) { m_positionSet = set; }

private:
    int m_id = -1;
    std::string m_name;
    float m_x = 0.0f;
    float m_y = 0.0f;
    bool m_positionSet = false;
};


#endif //GLITTER_NODE_HPP