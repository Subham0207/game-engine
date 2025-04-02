#include <3DModel/Animation/Timewarp.hpp>

namespace Animation3D{
    
float frameDistance(
    Animation* animation1,
    Animation* animation2,
    float relativeTime, 
    const AssimpNodeData* rootNode)
{    
    float totalDistance = 0.0f;
    calculateTransformationDistance(
        rootNode,
        animation1,
        animation2,
        relativeTime,
        relativeTime,
        totalDistance);
        
    return totalDistance;
}

void decomposeTransformation(const glm::mat4& mat, glm::vec3& position, glm::quat& rotation) {
    glm::vec3 scale, skew;
    glm::vec4 perspective;

    // Decompose the matrix into components
    glm::decompose(mat, scale, rotation, position, skew, perspective);

    // Normalize the rotation to ensure it is a unit quaternion
    rotation = glm::normalize(rotation);
}


void calculateTransformationDistance(
    const AssimpNodeData* node,
    Animation* animation1,
    Animation* animation2,
    float relativeTimeA,
    float relativeTimeB,
    float &totalDistance
    )
{
    std::string nodeName = node->name;
    
    glm::mat4 nodeTransform1 = node->transformation;
    glm::mat4 nodeTransform2 = node->transformation;
    
    if(animation1 && animation2)
    {
        auto animBone1 = animation1->FindBone(nodeName);
        auto animBone2 = animation2->FindBone(nodeName);
        if(animBone1 && animBone2)
        {
            animBone1->Update(relativeTimeA * animation1->GetDuration());
            animBone2->Update(relativeTimeB * animation2->GetDuration());
            nodeTransform1 = animBone1->GetLocalTransform();
            nodeTransform2 = animBone2->GetLocalTransform();
        }
    }
    
    glm::vec3 pos1, pos2;
    glm::quat rot1, rot2;

    // Decompose both transformation matrices
    decomposeTransformation(nodeTransform1, pos1, rot1);
    decomposeTransformation(nodeTransform2, pos2, rot2);

    // Calculate position distance using GLM's length function
    float posDist = glm::length(pos1 - pos2);

    // Calculate quaternion difference as the angle between the two rotations
    float rotDist = glm::angle(rot1 * glm::inverse(rot2));

    totalDistance += posDist + rotDist; // Total distance (position + rotation)

    for (int i = 0; i < node->childrenCount; i++)
    calculateTransformationDistance(node->children[i], animation1, animation2, relativeTimeA, relativeTimeB, totalDistance);
}

vector<pair<int, int>> dynamicTimeWarping(Animation* animation1, Animation* animation2, AssimpNodeData* rootNodeData) {
    int totalSteps = 10;

    vector<vector<double>> dtw(totalSteps + 1, vector<double>(totalSteps + 1, numeric_limits<double>::infinity()));
    vector<vector<pair<int, int>>> path(totalSteps + 1, vector<pair<int, int>>(totalSteps + 1));

    dtw[0][0] = 0.0;
    
    std::cout << "\n";
    std::cout << "Source: "<<animation1->animationName << " Target: " << animation2->animationName << std::endl;
    for (int i = 1; i <= totalSteps; ++i) {
        float relativeTimeA = i / float(totalSteps);
    
        for (int j = 1; j <= totalSteps; ++j) {
            float relativeTimeB = j / float(totalSteps);
    
            float frameCost = 0.0f;
            calculateTransformationDistance(
                rootNodeData,
                animation1,
                animation2,
                relativeTimeA,
                relativeTimeB,
                frameCost
            );

            float bestPrevCost = std::min({dtw[i - 1][j],    // Insertion
                dtw[i][j - 1],    // Deletion
                dtw[i - 1][j - 1] // Match
                });
    
            // Compute DTW cost matrix
            dtw[i][j] = frameCost + bestPrevCost;

            if (bestPrevCost == dtw[i-1][j]) {
                path[i][j] = {i-1, j};
            }
            else if (bestPrevCost == dtw[i][j-1]) {
                path[i][j] = {i, j-1};
            }
            else {
                path[i][j] = {i-1, j-1};
            }

        }
        std::cout << "\n";
    }
    
    std::cout << "dtw" << std::endl;
    for (int i = 0; i <= totalSteps; i++) {
            for (int j = 0; j <= totalSteps; j++) {
            std::cout << "| ";
            std::cout << dtw[i][j] << " ";
            std::cout << " |";
        }
        std::cout << "\n";
    }

    std::cout << "Path" << std::endl;
    for (int i = 0; i <= totalSteps; i++) {
            for (int j = 0; j <= totalSteps; j++) {
            std::cout << "| ";
            std::cout << path[i][j].first << ","<< path[i][j].second << " ";
            std::cout << " |";
        }
        std::cout << "\n";
    }

    // Backtracking to retrieve optimal path
    vector<pair<int, int>> alignmentPath;
    int i = totalSteps, j = totalSteps;
    while (i > 0 && j > 0) {
        alignmentPath.push_back({i, j});
        auto [prevI, prevJ] = path[i][j];
        i = prevI;
        j = prevJ;
    }

    std::cout << "Alignment curve" <<std::endl;
    for (size_t i = 0; i < alignmentPath.size(); i++)
    {
        std::cout << alignmentPath[i].first << ", "<< alignmentPath[i].second << std::endl;
    }
    
    reverse(alignmentPath.begin(), alignmentPath.end());

    return alignmentPath;
}

TimeWarpCurve* alignAnimations(
    Animation* sourceAnimation,
    Animation* targetAnimation,
    AssimpNodeData* rootNodeData
) {
    
    // Step 1: Perform DTW to get frame correspondences
    auto path = dynamicTimeWarping(sourceAnimation, targetAnimation, rootNodeData);
    
    return new TimeWarpCurve(path);
}


TimeWarpCurve::TimeWarpCurve(const vector<pair<int, int>>& path) {
    // Convert path to time points
    for (const auto& p : path) {
        sourceTimes.push_back(p.first);
        targetTimes.push_back(p.second);
    }
    
    // Fit cubic spline
    fitSpline();
}

float TimeWarpCurve::evaluate(float sourceTime) const {
    // Find the appropriate segment
    auto it = upper_bound(sourceTimes.begin(), sourceTimes.end(), sourceTime);
    if (it == sourceTimes.begin()) return targetTimes.front();
    if (it == sourceTimes.end()) return targetTimes.back();
    
    size_t idx = distance(sourceTimes.begin(), it) - 1;
    
    // Cubic interpolation
    float t = (sourceTime - sourceTimes[idx]) / 
             (sourceTimes[idx+1] - sourceTimes[idx]);
    t = max(0.0f, min(1.0f, t));
    
    return targetTimes[idx] * (1-t) + targetTimes[idx+1] * t;
}

void TimeWarpCurve::fitSpline() {
    // Simple cubic spline fitting (could be enhanced)
    // For production code, consider using a proper spline library
    coefficients.resize(sourceTimes.size());
    
    // Just copy target times for this simple example
    // In a real implementation, you'd solve a tridiagonal system
    // to get proper cubic spline coefficients
    coefficients = targetTimes;
}

}