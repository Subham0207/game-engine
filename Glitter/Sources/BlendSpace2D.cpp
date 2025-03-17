#include <Controls/BlendSpace2D.hpp>
#include <iostream>

BlendSelection BlendSpace2D::GetBlendSelection(glm::vec2 input) {
    BlendSelection result{nullptr, nullptr, nullptr, nullptr, 0.0f, 0.0f, 0.0f, 0.0f};

    if (blendPoints.empty()) return result; // No animations available

    // Find the four nearest blend points

    float minDistTL = FLT_MAX, minDistTR = FLT_MAX, minDistBL = FLT_MAX, minDistBR = FLT_MAX;
    bool hasTL = false, hasTR = false, hasBL = false, hasBR = false;

    BlendPoint topLeft = BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    BlendPoint topRight = BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    BlendPoint bottomLeft = BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    BlendPoint bottomRight = BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    for (auto& point : blendPoints) {
        float distance = glm::length(point.position - input);

        if (point.position.x <= input.x && point.position.y >= input.y && distance < minDistTL) {
            minDistTL = distance;
            topLeft = point;
            hasTL = true;
        }

        if (point.position.x >= input.x && point.position.y >= input.y && distance < minDistTR) {
            minDistTR = distance;
            topRight = point;
            hasTR = true;
        }

        if (point.position.x <= input.x && point.position.y <= input.y && distance < minDistBL) {
            minDistBL = distance;
            bottomLeft = point;
            hasBL = true;
        }

        if (point.position.x >= input.x && point.position.y <= input.y && distance < minDistBR) {
            minDistBR = distance;
            bottomRight = point;
            hasBR = true;
        }
    }

    calculateBlendFactors(input, result, topLeft, topRight, bottomLeft, bottomRight);
    
    result.bottomLeft = bottomLeft.animation;
    result.bottomRight = bottomRight.animation;
    result.topLeft = topLeft.animation;
    result.topRight = topRight.animation;

    return result;
}

void BlendSpace2D::calculateBlendFactors(
    glm::vec2 input,
    BlendSelection& result,
    BlendPoint anim1Point,
    BlendPoint anim2Point,
    BlendPoint anim3Point,
    BlendPoint anim4Point
)
{
    float d1 = glm::distance(input, anim1Point.position);
    float d2 = glm::distance(input, anim2Point.position);
    float d3 = glm::distance(input, anim3Point.position);
    float d4 = glm::distance(input, anim4Point.position)
    ;
    if (d1 == 0.0f || d2 == 0.0f || d3 == 0.0f || d4 == 0.0f) {
        result.topLeftBlendFactor = 1;
        result.topRightBlendFactor = 0;
        result.bottomLeftBlendFactor = 0;
        result.bottomRightBlendFactor = 0;
        return;
    }

    float totalDistance = d1 + d2 + d3 + d4;


    result.topLeftBlendFactor = (totalDistance - d1) / (3 * totalDistance);
    result.topRightBlendFactor = (totalDistance - d2) / (3 * totalDistance);
    result.bottomLeftBlendFactor = (totalDistance - d3) / (3 * totalDistance);
    result.bottomRightBlendFactor = (totalDistance - d4) / (3 * totalDistance);

}