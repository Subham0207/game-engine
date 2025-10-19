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

    // Camera forward in world space
    glm::mat4 invView = glm::inverse(view);
    glm::vec3 camForward = -glm::normalize(glm::vec3(invView[2])); // -Z of camera basis

    // 1) Try ground/hover plane at player Y
    float t; glm::vec3 hit;
    bool got = rayPlaneHit(
        O, D,
        glm::vec3(0.0f, playerPosition.y, 0.0f),
        glm::vec3(0,1,0),
        t, hit
    );

    // 2) If no good ground hit, use a tilted plane through the player that leans toward camera
    if (!got) {
        const float k = 0.6f; // 0..1 (0 = ground, 1 = camera-facing)
        glm::vec3 Ntilt = glm::normalize((1.0f - k) * glm::vec3(0,1,0) - k * camForward);
        got = rayPlaneHit(O, D, playerPosition, Ntilt, t, hit);

        // Extreme edge: if still no, nudge k a bit more aggressive
        if (!got) {
            glm::vec3 Ntilt2 = glm::normalize(0.3f * glm::vec3(0,1,0) - 0.7f * camForward);
            got = rayPlaneHit(O, D, playerPosition, Ntilt2, t, hit);
            if (!got) {
                // Last resort: look where the ray trends horizontally
                glm::vec3 dirXZ = glm::normalize(glm::vec3(D.x, 0.0f, D.z));
                float yawLR = std::atan2(dirXZ.x, dirXZ.z);
                return glm::angleAxis(yawLR, glm::vec3(0,1,0));
            }
        }
    }

    // 3) Build facing on XZ
    glm::vec3 dir = hit - playerPosition;
    dir.y = 0.0f;

    // Deadzone to avoid jitter if cursor is nearly above player
    if (glm::length2(dir) < 1e-8f) {
        // Keep current (or face camera forward projected)
        glm::vec3 f = glm::normalize(glm::vec3(camForward.x, 0.0f, camForward.z));
        float yawF = std::atan2(f.x, f.z);
        return glm::angleAxis(yawF, glm::vec3(0,1,0));
    }

    dir = glm::normalize(dir);

    // If your model's forward is +Z:
    float yaw = std::atan2(dir.x, dir.z);
    return glm::angleAxis(yaw, glm::vec3(0,1,0));
}