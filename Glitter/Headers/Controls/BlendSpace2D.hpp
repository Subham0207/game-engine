#pragma once
#include <3DModel/Animation/Animation.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <3DModel/Animation/Timewarp.hpp>
#include <serializeAClass.hpp>
#include <Serializable.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>

struct BlendPoint {
    glm::vec2 position;  // (X, Y) coordinates in the blend space

    std::string animationGuid;
    Animation* animation; // Animation assigned to this point

    int blendPointIndex = 0;

    BlendPoint()=default;
    BlendPoint(glm::vec2 pos, Animation* animation){
        animationGuid = animation ? animation->getAssetId(): "";
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

class BlendSpace2D: public Serializable {
public:
    BlendSpace2D()=default;
    BlendSpace2D(std::string blendspaceName){
        blendPoints = std::vector<BlendPoint>();
        this->blendspaceName = blendspaceName;
        generate_asset_guid();
    };
    
    void AddBlendPoint(glm::vec2 pos, Animation* anim) {
        blendPoints.push_back(BlendPoint(pos, anim) );
    }

    BlendSelection* GetBlendSelection(glm::vec2 input);

    void generateTimeWarpCurve(std::shared_ptr<AssimpNodeData> rootNode, std::map<std::pair<int,int>, Animation3D::TimeWarpCurve*> &timewarpCurveMap);
    std::vector<BlendPoint> blendPoints;

    std::string blendspaceName;
protected:
    virtual const std::string typeName() const override {return "blendspace"; }
    virtual const std::string contentName() override {return blendspaceName; }

    virtual void saveContent(fs::path contentFileLocation, std::ostream& os) override;
    virtual void loadContent(fs::path contentFileLocation, std::istream& is) override;

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
        ar & blendspaceName;
    }
};
