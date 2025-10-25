#pragma once
#include <vector>
#include <imgui.h>
#include <Controls/BlendSpace2D.hpp>

namespace UI
{
    bool ImGuiGrid2D(const std::vector<BlendPoint>& points, ImVec2* scrubbedPoint, const ImVec2& gridSize,const BlendSelection* selection, float pointRadius = 3.0f, float scrubRadius = 6.0f);

    class Blendspace2DUI
    {
        public:
            Blendspace2DUI()
            {
                scrubbedPoint = ImVec2(0,0);
                showBlendspaceUI = false;
            }
            void draw(BlendSpace2D *blendspace, BlendSelection* selection, bool &showUI);
            ImVec2 scrubbedPoint;
            bool showBlendspaceUI;
    };
}