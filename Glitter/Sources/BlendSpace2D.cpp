#include <Controls/BlendSpace2D.hpp>

BlendSelection BlendSpace2D::GetBlendSelection(glm::vec2 input) {
    BlendSelection result{nullptr, nullptr, nullptr, nullptr, 0.0f, 0.0f};

    if (blendPoints.empty()) return result; // No animations available

    // Find the four nearest blend points

    float minDistTL = FLT_MAX, minDistTR = FLT_MAX, minDistBL = FLT_MAX, minDistBR = FLT_MAX;

    BlendPoint topLeft = BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    BlendPoint topRight = BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    BlendPoint bottomLeft = BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    BlendPoint bottomRight = BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    for (auto& point : blendPoints) {
        float distance = glm::length(point.position - input);

        if (point.position.x <= input.x && point.position.y >= input.y && distance < minDistTL) {
            minDistTL = distance;
            topLeft = point;
        }

        if (point.position.x >= input.x && point.position.y >= input.y && distance < minDistTR) {
            minDistTR = distance;
            topRight = point;
        }

        if (point.position.x <= input.x && point.position.y <= input.y && distance < minDistBL) {
            minDistBL = distance;
            bottomLeft = point;
        }

        if (point.position.x >= input.x && point.position.y <= input.y && distance < minDistBR) {
            minDistBR = distance;
            bottomRight = point;
        }
    }

    // Compute blend factors
    auto xFactorDenominator = (bottomRight.position.x - bottomLeft.position.x);
    if(xFactorDenominator != 0)
    result.xFactor = (input.x - bottomLeft.position.x) / (bottomRight.position.x - bottomLeft.position.x);
    else
    result.xFactor = (input.x - bottomLeft.position.x) / 1;

    auto yFactorDenominator = (topLeft.position.y - bottomLeft.position.y);
    if(yFactorDenominator != 0)
    result.yFactor = (input.y - bottomLeft.position.y) / (topLeft.position.y - bottomLeft.position.y);
    else
    result.yFactor = (input.y - bottomLeft.position.y) / 1;


    result.bottomLeft = bottomLeft.animation;
    result.bottomRight = bottomRight.animation;
    result.topLeft = topLeft.animation;
    result.topRight = topRight.animation;

    result.xFactor = input.x;
    result.yFactor = input.y;

    return result;
}