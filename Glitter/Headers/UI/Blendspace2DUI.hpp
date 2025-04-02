#pragma once
#include <vector>
#include <imgui.h>
#include <Controls/BlendSpace2D.hpp>

namespace ProjectAssets
{
    bool ImGuiGrid2D(const std::vector<BlendPoint>& points, ImVec2* scrubbedPoint, const ImVec2& gridSize,const BlendSelection* selection, float pointRadius = 3.0f, float scrubRadius = 6.0f);

}