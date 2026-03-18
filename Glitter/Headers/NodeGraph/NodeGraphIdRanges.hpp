//
// Created by subha on 18-03-2026.
//
// Central place to allocate non-overlapping ImNodes integer ID ranges.
// All node types that are drawn inside the same ImNodes::BeginNodeEditor()
// MUST use globally unique IDs.
//
#ifndef GLITTER_NODEGRAPH_IDRANGES_HPP
#define GLITTER_NODEGRAPH_IDRANGES_HPP

#include <cstdint>

// Reserve fixed blocks for each UI element kind.
// If you add a new element type later, give it a new base = previous base + kBlockSize.
//
// Note: We intentionally keep these as ints because ImNodes APIs take int IDs.
static constexpr int kNodeGraphIdBlockSize = 1000;

enum class NodeGraphElementIdBase : int
{
	NodeGraphNode = 0,
	StateMachineNode = 1000,
	// Future:
	// SomethingElse = 2000,
};

// Helper: returns the base integer for an element kind.
[[nodiscard]] static constexpr int NodeGraphIdBase(NodeGraphElementIdBase base)
{
	return static_cast<int>(base);
}

#endif // GLITTER_NODEGRAPH_IDRANGES_HPP

