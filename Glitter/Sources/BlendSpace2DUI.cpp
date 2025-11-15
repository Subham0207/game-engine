#include "UI/Blendspace2DUI.hpp"
#include <Character/Character.hpp>
#include <EngineState.hpp>
#include <UI/Shared/ComboUI.hpp>
#include <UI/Shared/InputText.hpp>

UI::Grid2DResult UI::ImGuiGrid2D(
    std::vector<BlendPoint>& points,
    ImVec2* scrubbedPoint,
    const ImVec2& gridSize,
    const BlendSelection* selection,
    float pointRadius,
    float scrubRadius
) {
    Grid2DResult out;

    auto draggingPointIndex = &getUIState().blendspace2DUIState->draggingPointIndex;
    auto scrubberActive = getUIState().blendspace2DUIState->scrubberActive;

    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasMax = ImVec2(canvasPos.x + gridSize.x, canvasPos.y + gridSize.y);

    // The interactive area
    ImGui::InvisibleButton("gridCanvas", gridSize, ImGuiButtonFlags_MouseButtonLeft);
    bool isMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool isDragging = ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left);
    out.isScrubbing = isDragging;

    float scale = 50.0f;

    // Centered coords (your current mapping)
    ImVec2 canvasCenter = ImVec2(canvasPos.x + gridSize.x * 0.5f, canvasPos.y + gridSize.y * 0.5f);

    // Draw axes
    drawList->AddLine(ImVec2(canvasPos.x, canvasCenter.y), ImVec2(canvasMax.x, canvasCenter.y), IM_COL32(200,200,200,255));
    drawList->AddLine(ImVec2(canvasCenter.x, canvasPos.y), ImVec2(canvasCenter.x, canvasMax.y), IM_COL32(200,200,200,255));

    // Grid lines
    for (float x = -gridSize.x * 0.5f; x <= gridSize.x * 0.5f; x += scale) {
        drawList->AddLine(ImVec2(canvasCenter.x + x, canvasPos.y), ImVec2(canvasCenter.x + x, canvasMax.y), IM_COL32(100,100,100,255));
    }
    for (float y = -gridSize.y * 0.5f; y <= gridSize.y * 0.5f; y += scale) {
        drawList->AddLine(ImVec2(canvasPos.x, canvasCenter.y + y), ImVec2(canvasMax.x, canvasCenter.y + y), IM_COL32(100,100,100,255));
    }

    ImVec2 mouseScreen = io.MousePos;
    float hitRadius = pointRadius;
    int index = 0;

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        *draggingPointIndex = -1;
    }

    // Points
    for (auto& point : points) {
        ImVec2 screenPoint(
            canvasCenter.x + point.position.x * scale,
            canvasCenter.y - point.position.y * scale
        );

        float dx = mouseScreen.x - screenPoint.x;
        float dy = mouseScreen.y - screenPoint.y;
        float distSq = dx * dx + dy * dy;

        // Check if mouse is near the point (Hit Test)
        bool isMouseOverPoint = distSq < (hitRadius * hitRadius);

        if(isMouseOverPoint && isMouseDown && !scrubberActive)
        {
            *draggingPointIndex = index;
        }
        
        if (*draggingPointIndex == index) {
            
            // Update the *point's world position* with the *world position of the mouse*

            point.position.x = (mouseScreen.x - canvasCenter.x) / scale;
            point.position.y = (canvasCenter.y - mouseScreen.y) / scale; // note the flipped Y

            screenPoint.x = canvasCenter.x + point.position.x * scale;
            screenPoint.y = canvasCenter.y - point.position.y * scale;

        }

        if (screenPoint.x >= canvasPos.x && screenPoint.x <= canvasMax.x &&
            screenPoint.y >= canvasPos.y && screenPoint.y <= canvasMax.y) {

            drawList->AddCircleFilled(screenPoint, pointRadius, IM_COL32(255,255,255,255));
            
            //Blendselection: points taken for blending in resultant animation.
            if (selection) {
                char label[32];
                float blendValue = -1.0f;
                if (selection->bottomLeft  == &point) blendValue = selection->bottomLeftBlendFactor;
                else if (selection->bottomRight == &point) blendValue = selection->bottomRightBlendFactor;
                else if (selection->topLeft == &point) blendValue = selection->topLeftBlendFactor;
                else if (selection->topRight == &point) blendValue = selection->topRightBlendFactor;

                if (blendValue >= 0.0f) {
                    snprintf(label, sizeof(label), "%.2f", blendValue);
                    ImVec2 textSize = ImGui::CalcTextSize(label);
                    ImVec2 textPos(screenPoint.x - textSize.x * 0.5f, screenPoint.y + pointRadius + 2.0f);
                    drawList->AddText(textPos, IM_COL32(255,255,0,255), label);
                }
            }
        }

        index++;
    }

    // Initialize scrubbed point if needed
    if (scrubbedPoint->x == 0.0f && scrubbedPoint->y == 0.0f && !points.empty()) {
        scrubbedPoint->x = points[0].position.x;
        scrubbedPoint->y = points[0].position.y;
    }

    // Scrub (drag the red dot)
    if (isDragging && scrubberActive) {
        ImVec2 localMouse = io.MousePos;
        localMouse.x = (localMouse.x - canvasCenter.x) / scale;
        localMouse.y = (canvasCenter.y - localMouse.y) / scale; // flip y
        *scrubbedPoint = localMouse;
    }

    // Draw scrubbed
    ImVec2 screenScrub(
        canvasCenter.x + scrubbedPoint->x * scale,
        canvasCenter.y - scrubbedPoint->y * scale
    );
    if (screenScrub.x >= canvasPos.x && screenScrub.x <= canvasMax.x &&
        screenScrub.y >= canvasPos.y && screenScrub.y <= canvasMax.y) {
        drawList->AddCircleFilled(screenScrub, scrubRadius, IM_COL32(255,0,0,255));
    }

    // ---- DRAG & DROP TARGET ----
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ANIM_ASSET")) {
            // Payload is a zero-terminated GUID string
            const char* guid = static_cast<const char*>(payload->Data);
            if (guid && payload->DataSize > 0) {
                // Convert current mouse position to grid space (centered, scaled)
                ImVec2 localMouse = io.MousePos;
                localMouse.x = (localMouse.x - canvasCenter.x) / scale;
                localMouse.y = (canvasCenter.y - localMouse.y) / scale;

                out.hasDrop = true;
                out.dropPos = localMouse;
                out.droppedAnimGuid.assign(guid);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // (Optional) While dragging, preview where it will drop
    if (ImGui::GetDragDropPayload() && ImGui::IsItemHovered()) {
        ImVec2 previewLocal = io.MousePos;
        previewLocal.x = (previewLocal.x - canvasCenter.x) / scale;
        previewLocal.y = (canvasCenter.y - previewLocal.y) / scale;

        ImVec2 previewScreen(
            canvasCenter.x + previewLocal.x * scale,
            canvasCenter.y - previewLocal.y * scale
        );
        drawList->AddCircle(previewScreen, scrubRadius + 4.0f, IM_COL32(0,255,0,200), 24, 2.0f);
    }

    return out;
}
void UI::Blendspace2DUI::draw(BlendSpace2D* blendspace, BlendSelection* selection, bool & showUI)
{
    if(ImGui::Begin("Blendspace", &showUI))
    {
        UI::Shared::InputText("##NameBlendspace", blendspace->blendspaceName);
        if(ImGui::Button("Save"))
        {
            auto loc = fs::path(EngineState::state->currentActiveProjectDirectory) / "Assets";
            blendspace->save(loc);
        }

        ImGui::Checkbox("Scrubber active", &getUIState().blendspace2DUIState->scrubberActive);

        // Left: grid, Right: animations
        ImGui::Columns(2, nullptr, true);

        // --- Column 1: Grid ---
        ImVec2 gridSize = ImVec2(300, 300);
        auto result = UI::ImGuiGrid2D(
            blendspace->blendPoints,
            &getUIState().blendspace2DUIState->scrubbedPoint,
            gridSize,
            selection,
            /*pointRadius*/5.0f,
            /*scrubRadius*/6.0f
        );

        // Optional clamp (note glm::clamp returns a value; assign back!)
        getUIState().blendspace2DUIState->scrubbedPoint.x =
            glm::clamp(getUIState().blendspace2DUIState->scrubbedPoint.x, -2.0f, 2.0f);
        getUIState().blendspace2DUIState->scrubbedPoint.y =
            glm::clamp(getUIState().blendspace2DUIState->scrubbedPoint.y, -2.0f, 2.0f);

        // If something was dropped on the grid, create a point
        if (result.hasDrop && !result.droppedAnimGuid.empty()) {
            // Clamp drop if you want limits
            ImVec2 p = result.dropPos;
            p.x = glm::clamp(p.x, -2.0f, 2.0f);
            p.y = glm::clamp(p.y, -2.0f, 2.0f);

            // Create and add a new BlendPoint
            BlendPoint bp;
            bp.position = { p.x, p.y };
            bp.animationGuid = result.droppedAnimGuid; // make sure your BlendPoint has this field
            // if you also store a human-readable name, look it up from the map and set it here
            blendspace->blendPoints.push_back(std::move(bp));
        }

        ImGui::NextColumn();

        // --- Column 2: Animations (drag source) ---
        // Keep your old combo if you still want quick selection, but add a list for drag
        std::vector<std::pair<std::string,std::string>> animLinear;
        getUIState().blendspace2DUIState->selectedAnimationIndex =
            UI::Blendspace2DUI::AnimationsListDragSource(
                "Animations (drag to grid)",
                getUIState().blendspace2DUIState->selectedAnimationIndex,
                EngineState::state->engineRegistry->animationsFileMap,
                &animLinear
            );


        ImGui::Columns(1);
    }
    ImGui::End();
}

int UI::Blendspace2DUI::AnimationsListDragSource(
    const char* label,
    int selectedIndex,
    const std::map<std::string, std::string>& animationsFileMap,
    std::vector<std::pair<std::string,std::string>>* outLinear // optional: keep a linear order (guid,name)
) {
    // Create a stable linear view, since map iteration order is lexical by key.
    std::vector<std::pair<std::string,std::string>> items;
    items.reserve(animationsFileMap.size());
    for (auto& kv : animationsFileMap) {
        items.emplace_back(kv.first, kv.second); // (guid, name/path)
    }
    if (outLinear) *outLinear = items;

    ImGui::TextUnformatted(label);
    ImGui::BeginChild("##anim-list", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (int i = 0; i < (int)items.size(); ++i) {
        const std::string& guid = items[i].first;
        const std::string& display = items[i].second; // show filename or a prettified name

        ImGui::PushID(i);
        bool selected = (i == selectedIndex);
        if (ImGui::Selectable(display.c_str(), selected)) {
            selectedIndex = i;
        }

        // Begin drag source for this item
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            // Send GUID as zero-terminated string
            ImGui::SetDragDropPayload("ANIM_ASSET", guid.c_str(), (int)guid.size()+1);
            ImGui::Text("Dragging: %s", display.c_str());
            ImGui::EndDragDropSource();
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    return selectedIndex;
}