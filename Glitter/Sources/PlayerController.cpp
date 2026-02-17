#include <Controls/PlayerController.hpp>
#include <Helpers/raypicking.hpp>
#include <Modals/CameraType.hpp>
#include <Character/Character.hpp>
#include "EngineState.hpp"

Controls::PlayerController::PlayerController(std::string filename)
    : movementSpeed(0.0f), targetSpeed(0.0f), movementDirection(0.0f), targetDirection(0.0f),
        isJumping(false), grounded(false), dodgeStart(false), interpolationSpeed(0.1f), directionVector(0.0f,0.0f,0.0f), inputXWorld(0.0f),
        inputZWorld(0.0f), lookDirection(0.0f,0.0f,0.0f), filename(filename), isAiming(false), inputVectorLength(0.0f),
        characterRotation(0.0f), characterPosition(0.0f,0.0f,0.0f)
{
    cameraType = CameraType::TOP_DOWN;

    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    auto vertPath = engineFSPath / "Shaders/rayCast.vert";
    auto fragPath = engineFSPath / "Shaders/rayCast.frag";
    new Shader(vertPath.u8string().c_str(), fragPath.u8string().c_str());
}

void Controls::PlayerController::setMovement(glm::vec3 dir)
{
    if(isAiming)
    {
        if(glm::length(dir) > 0.00001f)
        {
            targetSpeed = dir.z;
            targetDirection = -dir.x;
        }
        else
        {
            targetSpeed = 0.0f;
            targetDirection = 0.0f;
        }
    }
    else
    {
        if(glm::length(dir) > 0.00001f)
        {
            targetSpeed = 1.0f; // to always return walking forward blendpoint when not aiming.
            targetDirection = 0.0f;
        }
        else
        {
            targetSpeed = 0.0f;
            targetDirection = 0.0f;
        }
    }

    inputXWorld = dir.x;
    inputZWorld = dir.z;
}

std::shared_ptr<Character> Controls::PlayerController::getCharacter()
{
    return this->character;
}

void Controls::PlayerController::setCharacter(const std::shared_ptr<Character>& characterRef)
{
    this->character = characterRef;
}

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
