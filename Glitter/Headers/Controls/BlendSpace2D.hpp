#pragma once
#include <3DModel/Animation/Animation.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <3DModel/Animation/Timewarp.hpp>
#include <serializeAClass.hpp>

struct BlendPoint {
    glm::vec2 position;  // (X, Y) coordinates in the blend space

    std::string animationGuid;
    Animation* animation; // Animation assigned to this point

    int blendPointIndex = 0;

    BlendPoint(glm::vec2 pos, Animation* animation){
        animationGuid = animation->getGUID();
        position = pos;
        this->animation = animation;
    }

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & position;
            ar & animationGuid;
        }
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
        blendPoints.push_back(BlendPoint(pos, anim) );
    }

    BlendSelection* GetBlendSelection(glm::vec2 input);

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

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
		ar & blendPoints;
    }
};
