#include <Controls/PlayerController.hpp>
#include <Helpers/raypicking.hpp>

glm::quat Controls::PlayerController::faceMouseOnXZ(
    glm::vec3& playerPosition,
    float mouseX, float mouseY,
    glm::mat4& view,glm::mat4& proj
)
{
    glm::vec3 O, D;
    setRay(mouseX, mouseY, O, D, view, proj, glm::vec3(0.0f));

    // Camera forward in world space:
    glm::mat4 invView = glm::inverse(view);
    // In GLM (column-major), basis vectors are columns; +Z is column 2.
    // Cameras look -Z in view space, so world forward is -invView[2].xyz
    glm::vec3 camForward = -glm::normalize(glm::vec3(invView[2]));

    // Plane through the player, parallel to the camera’s image plane
    float t; glm::vec3 hit;
    if (!intersectRayPlane_pointNormal(O, D, playerPosition, camForward, t, hit)) {
        // Super rare (ray parallel to plane). Fall back to camera forward.
        glm::vec3 fd = glm::normalize(glm::vec3(camForward.x, 0.0f, camForward.z));
        float yaw = std::atan2(fd.x, fd.z);
        return glm::angleAxis(yaw, glm::vec3(0,1,0));
    }

    // Build facing on XZ
    glm::vec3 faceDir = hit - playerPosition;
    faceDir.y = 0.0f;

    // If the hit is almost directly above/below, blend in camera forward to avoid jitter:
    if (glm::length2(faceDir) < 1e-8f) {
        faceDir = glm::vec3(camForward.x, 0.0f, camForward.z);
    }

    faceDir = glm::normalize(faceDir);

    // If your model’s forward is +Z:
    float yaw = std::atan2(faceDir.x, faceDir.z);
    return glm::angleAxis(yaw, glm::vec3(0,1,0));
}