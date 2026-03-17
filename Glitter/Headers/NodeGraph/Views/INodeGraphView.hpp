//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_INODEGRAPHVIEW_HPP
#define GLITTER_INODEGRAPHVIEW_HPP

#include "../NodeGraphLayer.hpp"

struct NodeGraphRenderContext;

class INodeGraphView
{
public:
    virtual ~INodeGraphView() = default;

    [[nodiscard]] virtual NodeGraphLayer layer() const = 0;

    // Smaller draws earlier within the same layer.
    [[nodiscard]] virtual int priority() const { return 0; }

    virtual void draw(NodeGraphRenderContext& ctx) = 0;
};

#endif //GLITTER_INODEGRAPHVIEW_HPP


