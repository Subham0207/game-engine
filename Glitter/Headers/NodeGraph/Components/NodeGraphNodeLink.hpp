//
// Created by subha on 24-03-2026.
//
// Lightweight link model for NodeGraph nodes.
// Stored separately from ImNodes so the graph can be serialized/edited.
//

#ifndef GLITTER_NODEGRAPH_NODELINK_HPP
#define GLITTER_NODEGRAPH_NODELINK_HPP

class NodeGraphNodeLink
{
public:
	NodeGraphNodeLink() = default;
	NodeGraphNodeLink(int id, int startAttr, int endAttr)
		: m_id(id), m_startAttr(startAttr), m_endAttr(endAttr)
	{
	}

	[[nodiscard]] int id() const { return m_id; }
	[[nodiscard]] int startAttr() const { return m_startAttr; }
	[[nodiscard]] int endAttr() const { return m_endAttr; }

	void setStartAttr(int attr) { m_startAttr = attr; }
	void setEndAttr(int attr) { m_endAttr = attr; }

private:
	int m_id = -1;
	int m_startAttr = -1; // output pin/attribute id
	int m_endAttr = -1;   // input pin/attribute id
};

#endif // GLITTER_NODEGRAPH_NODELINK_HPP

