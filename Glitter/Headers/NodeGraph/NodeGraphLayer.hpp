//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_NODEGRAPH_LAYER_HPP
#define GLITTER_NODEGRAPH_LAYER_HPP

// Drawing order: smaller draws first (behind), larger draws later (in front).
enum class NodeGraphLayer : int
{
    Background = 0,
    Content = 100,
    Overlay = 200,
    Popup = 300
};

#endif //GLITTER_NODEGRAPH_LAYER_HPP

