//
// Created by subha on 16-03-2026.
//

#ifndef GLITTER_COMMENTBOX_HPP
#define GLITTER_COMMENTBOX_HPP

#include <imgui.h>

class CommentBox
{
public:
	static constexpr int kMaxText = 1024;

	int id = -1;
	ImVec2 posGrid{0.0f, 0.0f};   // top-left in grid space (pans with editor)
	ImVec2 size{220.0f, 140.0f};  // in pixels
	char text[kMaxText] = "Comment";
};


#endif //GLITTER_COMMENTBOX_HPP