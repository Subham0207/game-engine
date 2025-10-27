#include "UI/Blendspace2DUI.hpp"
#include <Character/Character.hpp>
#include <EngineState.hpp>

bool UI::ImGuiGrid2D(
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

    // Calculate the center of the canvas in screen coordinates.
    // This will be our new origin (0,0) for the grid.
    ImVec2 canvasCenter = ImVec2(canvasPos.x + gridSize.x / 2.0f, canvasPos.y + gridSize.y / 2.0f);

    // Draw grid lines and axes
    // The previous loop only drew grid lines in the positive quadrant.
    // We now draw them relative to the center.
    // It's a bit more involved to draw a full grid, but we can at least draw the axes.

    // X-Axis
    drawList->AddLine(ImVec2(canvasPos.x, canvasCenter.y), ImVec2(canvasMax.x, canvasCenter.y), IM_COL32(200, 200, 200, 255));
    // Y-Axis
    drawList->AddLine(ImVec2(canvasCenter.x, canvasPos.y), ImVec2(canvasCenter.x, canvasMax.y), IM_COL32(200, 200, 200, 255));

    // Optional: Draw sub-grid lines.
    // We can loop through coordinates relative to the center.
    for (float x = -gridSize.x / 2.0f; x <= gridSize.x / 2.0f; x += scale) {
        drawList->AddLine(ImVec2(canvasCenter.x + x, canvasPos.y), ImVec2(canvasCenter.x + x, canvasMax.y), IM_COL32(100, 100, 100, 255));
    }
    for (float y = -gridSize.y / 2.0f; y <= gridSize.y / 2.0f; y += scale) {
        drawList->AddLine(ImVec2(canvasPos.x, canvasCenter.y + y), ImVec2(canvasMax.x, canvasCenter.y + y), IM_COL32(100, 100, 100, 255));
    }

    // Draw points and optionally blend weights
    for (const auto& point : points) {
        // Updated coordinate mapping
        // We now add the world coordinates (scaled) to the canvas center.
        // We also flip the y-axis because ImGui's screen y-axis increases downwards.
        ImVec2 screenPoint = ImVec2(
            canvasCenter.x + point.position.x * scale,
            canvasCenter.y - point.position.y * scale
        );

        // This check is good for keeping points within the visible area.
        if (screenPoint.x >= canvasPos.x && screenPoint.x <= canvasMax.x &&
            screenPoint.y >= canvasPos.y && screenPoint.y <= canvasMax.y) {

            drawList->AddCircleFilled(screenPoint, pointRadius, IM_COL32(255, 255, 255, 255));

            // Show blend weights if this point is part of the current selection
            if (selection) {
                // ... (The rest of this block remains the same, it's correct)
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
    // This logic is fine, but you might want to handle an empty points vector gracefully.
    if (scrubbedPoint->x == 0.0f && scrubbedPoint->y == 0.0f && !points.empty()) {
        scrubbedPoint->x = points[0].position.x;
        scrubbedPoint->y = points[0].position.y;
    }

    // Handle dragging
    if (isDragging) {
        ImVec2 localMouse = io.MousePos;
        // The dragging logic also needs to be updated to be relative to the center.
        localMouse.x = (localMouse.x - canvasCenter.x) / scale;
        localMouse.y = (canvasCenter.y - localMouse.y) / scale; // Note the flip
        *scrubbedPoint = localMouse;
    }

    // Draw scrubbed point
    // This also needs the updated coordinate mapping.
    ImVec2 screenScrub = ImVec2(
        canvasCenter.x + scrubbedPoint->x * scale,
        canvasCenter.y - scrubbedPoint->y * scale
    );

    if (screenScrub.x >= canvasPos.x && screenScrub.x <= canvasMax.x &&
        screenScrub.y >= canvasPos.y && screenScrub.y <= canvasMax.y) {
        drawList->AddCircleFilled(screenScrub, scrubRadius, IM_COL32(255, 0, 0, 255));
    }

    return isDragging;
}
void UI::Blendspace2DUI::draw(BlendSpace2D* blendspace, BlendSelection* selection, bool & showUI)
{
    if(ImGui::Begin("Blendspace"))
    {
        ImVec2 gridSize = ImVec2(200, 200);
        if(UI::ImGuiGrid2D(blendspace->blendPoints, &getUIState().blendspace2DUIState->scrubbedPoint, gridSize, selection)){
            // ImGui::Text("Scrubbed Point: (%.2f, %.2f)", getUIState().scrubbedPoint.x, getUIState().scrubbedPoint.y);

            glm::clamp(getUIState().blendspace2DUIState->scrubbedPoint.x, -2.0f,2.0f);
            glm::clamp(getUIState().blendspace2DUIState->scrubbedPoint.y, -2.0f,2.0f);
        }
        else{
            // ImGui::Text("Scrubbed Point: (%.2f, %.2f)", getUIState().scrubbedPoint.x, getUIState().scrubbedPoint.y);
        }
    }
    ImGui::End();
}