#include "UI/Blendspace2DUI.hpp"

bool ProjectAssets::ImGuiGrid2D(
    const std::vector<BlendPoint>& points,
    ImVec2* scrubbedPoint,
    const ImVec2& gridSize,
    const BlendSelection* selection,
    float pointRadius,
    float scrubRadius
) {
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasMax = ImVec2(canvasPos.x + gridSize.x, canvasPos.y + gridSize.y);

    ImGui::InvisibleButton("gridCanvas", gridSize);
    bool isDragging = ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left);

    float scale = 50.0f;

    // Optional grid lines
    for (float x = canvasPos.x; x <= canvasMax.x; x += gridSize.x / 10.0f)
        drawList->AddLine(ImVec2(x, canvasPos.y), ImVec2(x, canvasMax.y), IM_COL32(100, 100, 100, 255));
    for (float y = canvasPos.y; y <= canvasMax.y; y += gridSize.y / 10.0f)
        drawList->AddLine(ImVec2(canvasPos.x, y), ImVec2(canvasMax.x, y), IM_COL32(100, 100, 100, 255));

    // Draw points and optionally blend weights
    for (const auto& point : points) {
        ImVec2 screenPoint = ImVec2(
            canvasPos.x + point.position.x * scale,
            canvasMax.y - point.position.y * scale
        );

        if (screenPoint.x >= canvasPos.x && screenPoint.x <= canvasMax.x &&
            screenPoint.y >= canvasPos.y && screenPoint.y <= canvasMax.y) {

            drawList->AddCircleFilled(screenPoint, pointRadius, IM_COL32(255, 255, 255, 255));

            // Show blend weights if this point is part of the current selection
            if (selection) {
                char label[32];
                float blendValue = -1.0f;

                if (selection->bottomLeft == &point) {
                    blendValue = selection->bottomLeftBlendFactor;
                } else if (selection->bottomRight == &point) {
                    blendValue = selection->bottomRightBlendFactor;
                } else if (selection->topLeft == &point) {
                    blendValue = selection->topLeftBlendFactor;
                } else if (selection->topRight == &point) {
                    blendValue = selection->topRightBlendFactor;
                }

                if (blendValue >= 0.0f) {
                    snprintf(label, sizeof(label), "%.2f", blendValue);
                    ImVec2 textSize = ImGui::CalcTextSize(label);
                    ImVec2 textPos = ImVec2(screenPoint.x - textSize.x / 2.0f, screenPoint.y + pointRadius + 2.0f);
                    drawList->AddText(textPos, IM_COL32(255, 255, 0, 255), label);
                }
            }
        }
    }

    // Initialize scrubbed point
    if (scrubbedPoint->x == 0.0f && scrubbedPoint->y == 0.0f && !points.empty()) {
        scrubbedPoint->x = points[0].position.x;
        scrubbedPoint->y = points[0].position.y;
    }

    // Handle dragging
    if (isDragging) {
        ImVec2 localMouse = io.MousePos;
        localMouse.x = (localMouse.x - canvasPos.x) / scale;
        localMouse.y = (canvasMax.y - localMouse.y) / scale;
        *scrubbedPoint = localMouse;
    }

    // Draw scrubbed point
    ImVec2 screenScrub = ImVec2(
        canvasPos.x + scrubbedPoint->x * scale,
        canvasMax.y - scrubbedPoint->y * scale
    );

    if (screenScrub.x >= canvasPos.x && screenScrub.x <= canvasMax.x &&
        screenScrub.y >= canvasPos.y && screenScrub.y <= canvasMax.y) {
        drawList->AddCircleFilled(screenScrub, scrubRadius, IM_COL32(255, 0, 0, 255));
    }

    return isDragging;
}
