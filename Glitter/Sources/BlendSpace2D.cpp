#include <Controls/BlendSpace2D.hpp>
#include <iostream>
#include <vector>

BlendSelection BlendSpace2D::GetBlendSelection(glm::vec2 input) {
    BlendSelection result{nullptr, nullptr, nullptr, nullptr, 0.0f, 0.0f, 0.0f, 0.0f};

    if (blendPoints.empty()) return result; // No animations available

    // Find the four nearest blend points

    float minDistTL = FLT_MAX, minDistTR = FLT_MAX, minDistBL = FLT_MAX, minDistBR = FLT_MAX;
    bool hasTL = false, hasTR = false, hasBL = false, hasBR = false;

    auto topLeft = new BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    auto topRight = new BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    auto bottomLeft = new BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    auto bottomRight = new BlendPoint({glm::vec2(0.0f,0.0f), nullptr});
    for (auto& point : blendPoints) {
        float distance = glm::length(point.position - input);

        if (point.position.x <= input.x && point.position.y >= input.y && distance < minDistTL) {
            minDistTL = distance;
            topLeft = &point;
            hasTL = true;
        }

        if (point.position.x >= input.x && point.position.y >= input.y && distance < minDistTR) {
            minDistTR = distance;
            topRight = &point;
            hasTR = true;
        }

        if (point.position.x <= input.x && point.position.y <= input.y && distance < minDistBL) {
            minDistBL = distance;
            bottomLeft = &point;
            hasBL = true;
        }

        if (point.position.x >= input.x && point.position.y <= input.y && distance < minDistBR) {
            minDistBR = distance;
            bottomRight = &point;
            hasBR = true;
        }
    }

    calculateBlendFactors(input, result, *topLeft, *topRight, *bottomLeft, *bottomRight);
    
    result.bottomLeft = bottomLeft;
    result.bottomRight = bottomRight;
    result.topLeft = topLeft;
    result.topRight = topRight;

    return result;
}

void BlendSpace2D::generateTimeWarpCurve(AssimpNodeData* rootNode, std::map<std::pair<int,int>, Animation3D::TimeWarpCurve*> &timewarpCurveMap)
{
    for (size_t i = 0; i < blendPoints.size(); i++)
    {
        auto currentPoint = blendPoints[i];
        auto input = currentPoint.position;
        //Find 4 points to blend to: top left, top right, bottom left, bottom right.
        auto neigbours = std::vector<BlendPoint>(4, BlendPoint({glm::vec2(0.0f,0.0f), nullptr}));
        float minDistTL = FLT_MAX, minDistTR = FLT_MAX, minDistBL = FLT_MAX, minDistBR = FLT_MAX;
        for (size_t j = 0; j < blendPoints.size(); j++) {
            auto point = blendPoints[j];
            float distance = glm::length(point.position - input);
    
            if (point.position.x <= input.x && point.position.y >= input.y && distance < minDistTL
            && point.position != input) {
                minDistTL = distance;
                neigbours[0] = point;
                neigbours[0].blendPointIndex = j;
            }
            
            if (point.position.x >= input.x && point.position.y >= input.y && distance < minDistTR 
                && point.position != input) {
                minDistTR = distance;
                neigbours[1] = point;
                neigbours[1].blendPointIndex = j;
            }
    
            if (point.position.x <= input.x && point.position.y <= input.y && distance < minDistBL 
                && point.position != input) {
                minDistBL = distance;
                neigbours[2] = point;
                neigbours[2].blendPointIndex = j;
            }
    
            if (point.position.x >= input.x && point.position.y <= input.y && distance < minDistBR 
                && point.position != input) {
                minDistBR = distance;
                neigbours[3] = point;
                neigbours[3].blendPointIndex = j;
            }
        }
        
        std::cout << "current: "<< currentPoint.animation->animationName << std::endl;
        for (size_t j = 0; j < 4; j++)
        {
            if(i != j && neigbours[j].animation != NULL)
            {
                std::cout << "nighbour: "<< neigbours[j].animation->animationName << std::endl;
                auto timewarpCurve = Animation3D::alignAnimations(
                    currentPoint.animation,
                    neigbours[j].animation,
                    rootNode
                );
                timewarpCurveMap[{i, neigbours[j].blendPointIndex}] = timewarpCurve;
            }
        }
        
    }


}

void BlendSpace2D::calculateBlendFactors(
    glm::vec2 input,
    BlendSelection &result,
    BlendPoint anim1Point, // top left  (x0, y0)
    BlendPoint anim2Point, // top right (x1, y0)
    BlendPoint anim3Point, // bottom left (x0, y1)
    BlendPoint anim4Point) // bottom right (x1, y1)
{
    float x0 = anim1Point.position.x;
    float y0 = anim1Point.position.y;
    float x1 = anim2Point.position.x;
    float y1 = anim3Point.position.y;

    // Prevent divide-by-zero
    float dx = x1 - x0;
    float dy = y1 - y0;
    
    // Compute alpha and beta
    float alpha = 0.0f;
    float beta  = 0.0f;

    if (dx != 0.0f) {
        alpha = (input.x - x0) / dx;
        alpha = glm::clamp(alpha, 0.0f, 1.0f);
    } else {
        alpha = 0.0f;
    }

    if (dy != 0.0f) {
        beta  = (input.y - y0) / dy;
        beta  = glm::clamp(beta, 0.0f, 1.0f);
    } else {
        beta = 0.0f;
    }

    // Bilinear interpolation weights
    float tl = (1.0f - alpha) * (1.0f - beta);
    float tr = alpha * (1.0f - beta);
    float bl = (1.0f - alpha) * beta;
    float br = alpha * beta;

    result.topLeftBlendFactor = tl;
    result.topRightBlendFactor = tr;
    result.bottomLeftBlendFactor = bl;
    result.bottomRightBlendFactor = br;
}