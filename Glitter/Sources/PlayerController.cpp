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

    float t = -rayOrigin.y / rayDir.y;
    glm::vec3 mouseWorldPos = rayOrigin + t * rayDir;

    lookDirection = mouseWorldPos - playerPosition;
    lookDirection.y = 0; // Ensure it's flat on the XZ plane

    if (glm::length(lookDirection) > 0.01f) { // Avoid division by zero if mouse is exactly on player
        lookDirection = glm::normalize(lookDirection);

        // Calculate angle using atan2.
        // atan2(y, x) gives angle from positive X-axis to vector (x, y).
        // Our character faces positive Z. So, we want to align the positive Z axis
        // of the player's local space with the lookDirection.
        // The angle we need is from (0,0,1) to (lookDirection.x, 0, lookDirection.z).
        // atan2(z, x) here gives the angle from X to Z. Our character is initially facing Z.
        // So, playerRotationY = atan2(lookDirection.x, lookDirection.z);
        // This gives rotation around Y.
        auto playerRotationY = std::atan2(lookDirection.x, lookDirection.z);

        return glm::angleAxis(playerRotationY, glm::vec3(0,1,0));
    }
    return glm::angleAxis(0.0f, glm::vec3(0,1,0));
}