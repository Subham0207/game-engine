#pragma once
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <3DModel/Animation/Animation.hpp>

using namespace std;


namespace Animation3D{

    // Distance between two animation frames (considering joint rotations and positions)
    float frameDistance(
        Animation* animation1,
        Animation* animation2,
        float relativeTime, 
        const std::shared_ptr<AssimpNodeData> rootNode);

    void decomposeTransformation(const glm::mat4& mat, glm::vec3& position, glm::quat& rotation);

    void calculateTransformationDistance(
        const std::shared_ptr<AssimpNodeData> node,
        Animation* animation1,
        Animation* animation2,
        float relativeTimeA,
        float relativeTimeB,
        float &totalDistance
        );

    vector<pair<int, int>> dynamicTimeWarping(Animation* animation1, Animation* animation2, std::shared_ptr<AssimpNodeData> rootNodeData);

    // TimeWarpCurve class for smooth animation alignment
    class TimeWarpCurve {
    public:
        TimeWarpCurve(const vector<pair<int, int>>& path);
        
        // Evaluate curve at source time
        float evaluate(float sourceTime) const;
        
    private:
        vector<float> sourceTimes;
        vector<float> targetTimes;
        vector<float> coefficients;
        
        void fitSpline();
    };

    // Animation alignment function
    TimeWarpCurve* alignAnimations(
        Animation* sourceAnimation,
        Animation* targetAnimation,
        std::shared_ptr<AssimpNodeData> rootNodeData
    );

}