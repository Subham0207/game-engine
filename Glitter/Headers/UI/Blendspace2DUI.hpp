#pragma once
#include <vector>
#include <imgui.h>
#include <Controls/BlendSpace2D.hpp>

namespace UI
{
    struct Grid2DResult {
        bool isScrubbing = false;      // you already return this; keep it
        bool hasDrop = false;          // new: did we accept a payload this frame?
        ImVec2 dropPos = ImVec2(0,0);  // new: world/grid-space (your blendspace space)
        std::string droppedAnimGuid;   // new: payload (animation GUID)
    };

    Grid2DResult ImGuiGrid2D(const std::vector<BlendPoint>& points, ImVec2* scrubbedPoint, const ImVec2& gridSize,const BlendSelection* selection, float pointRadius = 3.0f, float scrubRadius = 6.0f);

    class Blendspace2DUI
    {
        public:
            Blendspace2DUI()
            {
                scrubbedPoint = ImVec2(0,0);
                showBlendspaceUI = false;
                UIOpenedForBlendspace = nullptr;
            }
            static void draw(BlendSpace2D *blendspace, BlendSelection* selection, bool &showUI);
            ImVec2 scrubbedPoint;
            bool showBlendspaceUI;
            BlendSpace2D* UIOpenedForBlendspace;
            int selectedAnimationIndex;
        private:
            static int AnimationsListDragSource(
                const char* label,
                int selectedIndex,
                const std::map<std::string, std::string>& animationsFileMap,
                std::vector<std::pair<std::string,std::string>>* outLinear // optional: keep a linear order (guid,name)
            );
    };
}