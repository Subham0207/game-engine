#include <Controls/PlayerController.hpp>
#include <Helpers/raypicking.hpp>

glm::quat Controls::PlayerController::faceMouseOnXZ(
    glm::vec3& playerPosition,
    float mouseX, float mouseY,
    glm::mat4& view,glm::mat4& proj
)
{
    glm::vec3 rayOrigin, rayDir;
    setRay(mouseX, mouseY, rayOrigin, rayDir, view, proj, glm::vec3(0.0f));

    // Ground/hover plane: y = playerPosition.y
    glm::vec3 planeNormal(0.0f, 1.0f, 0.0f);
    glm::vec3 planePoint (0.0f, playerPosition.y, 0.0f);

    float t; glm::vec3 hit;
    if (!intersectRayPlane(rayOrigin, rayDir, planeNormal, planePoint, t, hit)) {
        // No intersection in front of camera — keep current orientation
        return glm::angleAxis(0.0f, glm::vec3(0,1,0)); // or return your last rotation
    }

    glm::vec3 lookDirection = hit - playerPosition;
    lookDirection.y = 0.0f;

    if (glm::length2(lookDirection) < 1e-6f) {
        return glm::angleAxis(0.0f, glm::vec3(0,1,0));
    }

    lookDirection = glm::normalize(lookDirection);

    // If your model’s forward is +Z:
    float yaw = std::atan2(lookDirection.x, lookDirection.z);
    return glm::angleAxis(yaw, glm::vec3(0,1,0));
}