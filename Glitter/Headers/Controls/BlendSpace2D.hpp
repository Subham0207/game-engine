#pragma once
#include <3DModel/Animation/Animation.hpp>
#include <glm/glm.hpp>
#include <vector>

struct BlendPoint {
    glm::vec2 position;  // (X, Y) coordinates in the blend space
    Animation* animation; // Animation assigned to this point
};

struct BlendSelection {
    Animation* bottomLeft;
    Animation* bottomRight;
    Animation* topLeft;
    Animation* topRight;
    float xFactor;
    float yFactor;
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

private:
    std::vector<BlendPoint> blendPoints;
};
