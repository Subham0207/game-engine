#include <Controls/BlendSpace2D.hpp>
#include <iostream>
#include <vector>
#include <EngineState.hpp>
#include <filesystem>
#include <cmath>
namespace fs = std::filesystem;

void BlendSpace2D::interpolateToScrubberLocation(glm::vec2 loc){
    const float t = glm::clamp(interpolationSpeed, 0.0f, 1.0f);
    scrubberLocation = glm::mix(scrubberLocation, loc, t);

    // Snap when very close to avoid tiny floating-point drift near the target.
    const glm::vec2 remaining = loc - scrubberLocation;
    if ((remaining.x * remaining.x) + (remaining.y * remaining.y) < 1e-6f)
        scrubberLocation = loc;
}

BlendSelection* BlendSpace2D::GetBlendSelection() {
    auto result = new BlendSelection {nullptr, nullptr, nullptr, nullptr, 0.0f, 0.0f, 0.0f, 0.0f};

    if (blendPoints.empty()) return result; // No animations available

    // Find the four nearest blend points

    float minDistTL = FLT_MAX, minDistTR = FLT_MAX, minDistBL = FLT_MAX, minDistBR = FLT_MAX;
    BlendPoint* topLeft = nullptr;
    BlendPoint* topRight = nullptr;
    BlendPoint* bottomLeft = nullptr;
    BlendPoint* bottomRight = nullptr;

    BlendPoint* nearestPoint = nullptr;
    float nearestDistSq = FLT_MAX;

    for (auto& point : blendPoints) {
        const glm::vec2 d = point.position - scrubberLocation;
        const float distSq = d.x * d.x + d.y * d.y;
        const float distance = sqrtf(distSq);

        if (distSq < nearestDistSq) {
            nearestDistSq = distSq;
            nearestPoint = &point;
        }

        if (point.position.x <= scrubberLocation.x && point.position.y >= scrubberLocation.y && distance < minDistTL) {
            minDistTL = distance;
            topLeft = &point;
        }

        if (point.position.x >= scrubberLocation.x && point.position.y >= scrubberLocation.y && distance < minDistTR) {
            minDistTR = distance;
            topRight = &point;
        }

        if (point.position.x <= scrubberLocation.x && point.position.y <= scrubberLocation.y && distance < minDistBL) {
            minDistBL = distance;
            bottomLeft = &point;
        }

        if (point.position.x >= scrubberLocation.x && point.position.y <= scrubberLocation.y && distance < minDistBR) {
            minDistBR = distance;
            bottomRight = &point;
        }
    }

    // Stable fallback when a quadrant has no point.
    if (!topLeft) topLeft = nearestPoint;
    if (!topRight) topRight = nearestPoint;
    if (!bottomLeft) bottomLeft = nearestPoint;
    if (!bottomRight) bottomRight = nearestPoint;

    calculateBlendFactors(scrubberLocation, *result, *topLeft, *topRight, *bottomLeft, *bottomRight);
    
    result->bottomLeft = bottomLeft;
    result->bottomRight = bottomRight;
    result->topLeft = topLeft;
    result->topRight = topRight;

    return result;
}

void BlendSpace2D::generateTimeWarpCurve(std::shared_ptr<AssimpNodeData> rootNode, std::map<std::pair<int,int>, Animation3D::TimeWarpCurve*> &timewarpCurveMap)
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
    constexpr float kExactEpsSq = 1e-8f;
    // IDW power=2 (1/d^2) gives stronger local dominance than 1/d.
    constexpr float kInvDistEpsSq = 1e-8f;

    const glm::vec2 dTL = input - anim1Point.position;
    const glm::vec2 dTR = input - anim2Point.position;
    const glm::vec2 dBL = input - anim3Point.position;
    const glm::vec2 dBR = input - anim4Point.position;

    const float d2TL = dTL.x * dTL.x + dTL.y * dTL.y;
    const float d2TR = dTR.x * dTR.x + dTR.y * dTR.y;
    const float d2BL = dBL.x * dBL.x + dBL.y * dBL.y;
    const float d2BR = dBR.x * dBR.x + dBR.y * dBR.y;

    // Exact hit on any selected corner should produce deterministic one-hot output.
    if (d2TL <= kExactEpsSq) {
        result.topLeftBlendFactor = 1.0f;
        result.topRightBlendFactor = 0.0f;
        result.bottomLeftBlendFactor = 0.0f;
        result.bottomRightBlendFactor = 0.0f;
        return;
    }
    if (d2TR <= kExactEpsSq) {
        result.topLeftBlendFactor = 0.0f;
        result.topRightBlendFactor = 1.0f;
        result.bottomLeftBlendFactor = 0.0f;
        result.bottomRightBlendFactor = 0.0f;
        return;
    }
    if (d2BL <= kExactEpsSq) {
        result.topLeftBlendFactor = 0.0f;
        result.topRightBlendFactor = 0.0f;
        result.bottomLeftBlendFactor = 1.0f;
        result.bottomRightBlendFactor = 0.0f;
        return;
    }
    if (d2BR <= kExactEpsSq) {
        result.topLeftBlendFactor = 0.0f;
        result.topRightBlendFactor = 0.0f;
        result.bottomLeftBlendFactor = 0.0f;
        result.bottomRightBlendFactor = 1.0f;
        return;
    }

    const float wTL = 1.0f / glm::max(d2TL, kInvDistEpsSq);
    const float wTR = 1.0f / glm::max(d2TR, kInvDistEpsSq);
    const float wBL = 1.0f / glm::max(d2BL, kInvDistEpsSq);
    const float wBR = 1.0f / glm::max(d2BR, kInvDistEpsSq);

    const float sum = wTL + wTR + wBL + wBR;
    float tl = 0.0f;
    float tr = 0.0f;
    float bl = 0.0f;
    float br = 0.0f;
    if (sum > 0.0f) {
        tl = wTL / sum;
        tr = wTR / sum;
        bl = wBL / sum;
        br = wBR / sum;
    }

    result.topLeftBlendFactor = tl;
    result.topRightBlendFactor = tr;
    result.bottomLeftBlendFactor = bl;
    result.bottomRightBlendFactor = br;
}

void BlendSpace2D::saveContent(fs::path contentFile, std::ostream& os)
{
    fs::path dir = fs::path(contentFile.string()).parent_path();
    if (dir.empty()) {
        // Set the directory to the current working directory
        dir = fs::current_path();
    }
    if (!fs::exists(dir)) {
        if (!fs::create_directories(dir)) {
            std::cerr << "Failed to create directories: " << dir << std::endl;
            return;
        }
    }
    std::ofstream ofs(contentFile.string());
    boost::archive::text_oarchive oa(ofs);
    oa << *this;
    ofs.close();
}

void BlendSpace2D::loadContent(fs::path contentFile, std::istream& is)
{
    std::ifstream ifs(contentFile.string());
    boost::archive::text_iarchive ia(ifs);
    ia >> *this;

    auto filesMap = getEngineRegistryFilesMap();
    for (size_t i = 0; i < this->blendPoints.size(); i++)
    {
        auto animationGuid = blendPoints[i].animationGuid;
        auto animationLocation = fs::path(filesMap[animationGuid]);
        blendPoints[i].animation = new Animation();
        blendPoints[i].animation->load(animationLocation.parent_path(), animationGuid);
    }
    
}