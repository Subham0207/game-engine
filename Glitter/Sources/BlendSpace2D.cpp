#include <Controls/BlendSpace2D.hpp>

BlendSelection BlendSpace2D::GetBlendSelection(glm::vec2 input) {
    BlendSelection result{nullptr, nullptr, nullptr, nullptr, 0.0f, 0.0f};

    if (blendPoints.empty()) return result; // No animations available

    // Find the four nearest blend points
    BlendPoint topLeft = BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    BlendPoint topRight = BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    BlendPoint bottomLeft = BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    BlendPoint bottomRight = BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    for (const auto& point : blendPoints) {
        if (point.position.x <= input.x && point.position.y >= input.y) topLeft = point;
        if (point.position.x >= input.x && point.position.y >= input.y) topRight = point;
        if (point.position.x <= input.x && point.position.y <= input.y) bottomLeft = point;
        if (point.position.x >= input.x && point.position.y <= input.y) bottomRight = point;
    }

    if (!bottomLeft.animation || !bottomRight.animation || !topLeft.animation || !topRight.animation)
        return result; // Ensure valid animations

    // Compute blend factors
    result.xFactor = (input.x - bottomLeft.position.x) / (bottomRight.position.x - bottomLeft.position.x);
    result.yFactor = (input.y - bottomLeft.position.y) / (topLeft.position.y - bottomLeft.position.y);

    result.bottomLeft = bottomLeft.animation;
    result.bottomRight = bottomRight.animation;
    result.topLeft = topLeft.animation;
    result.topRight = topRight.animation;

    return result;
}