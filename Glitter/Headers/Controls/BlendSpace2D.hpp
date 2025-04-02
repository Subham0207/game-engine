#pragma once
#include <3DModel/Animation/Animation.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <3DModel/Animation/Timewarp.hpp>

struct BlendPoint {
    glm::vec2 position;  // (X, Y) coordinates in the blend space
    Animation* animation; // Animation assigned to this point
    int blendPointIndex = 0;
};

struct BlendSelection {
    BlendPoint* bottomLeft;
    BlendPoint* bottomRight;
    BlendPoint* topLeft;
    BlendPoint* topRight;

    float bottomLeftBlendFactor;
    float bottomRightBlendFactor;
    float topLeftBlendFactor;
    float topRightBlendFactor;
};

class BlendSpace2D {
public:
    BlendSpace2D(){
        blendPoints = std::vector<BlendPoint>();
    };
    
    void AddBlendPoint(glm::vec2 pos, Animation* anim) {
        blendPoints.push_back({ pos, anim });
    }

    BlendSelection GetBlendSelection(glm::vec2 input);

    void generateTimeWarpCurve(AssimpNodeData* rootNode, std::map<std::pair<int,int>, Animation3D::TimeWarpCurve*> &timewarpCurveMap);
    std::vector<BlendPoint> blendPoints;

private:
    void calculateBlendFactors(
        glm::vec2 input,
        BlendSelection& result,
        BlendPoint anim1Point,
        BlendPoint anim2Point,
        BlendPoint anim3Point,
        BlendPoint anim4Point
    );
};
