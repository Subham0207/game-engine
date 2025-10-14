#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/quaternion.hpp>
#include <LuaEngine/LuaEngine.hpp>

namespace Controls
{
    class PlayerController
    {        
    public:
        PlayerController(std::string filename)
            : movementSpeed(0.0f), targetSpeed(0.0f), movementDirection(0.0f), targetDirection(0.0f),
              isJumping(false), grounded(false), dodgeStart(false), interpolationSpeed(0.1f), directionVector(0.0f,0.0f,0.0f), inputXWorld(0.0f),
              inputZWorld(0.0f), lookDirection(0.0f,0.0f,0.0f), filename(filename)
        {}

        float movementSpeed = 0.0f;           // Current speed (blended)
        float targetSpeed = 0.0f;             // Target speed (where we want to go)
        float movementDirection = 0.0f;   // Current direction (blended)
        float targetDirection= 0.0f;     // Target direction (where we want to go)
        bool isJumping;
        bool grounded;
        bool dodgeStart;
        float interpolationSpeed;      // Controls how fast blending happens (0.1 = smooth, 1.0 = instant)

        glm::vec3 lookDirection;

        glm::vec3 directionVector;

        glm::vec3 forwardVector;
        glm::vec3 rightVector;
        glm::quat characterRotation;
        glm::mat4 modelTransform;

        float inputXWorld;
        float inputZWorld;

        std::string filename;

        void setMovement(glm::vec3 dir)
        {
            // The input here is actually normalized vector in world space from character position.
            // so input alone should not decide which animation to play
            // compare  input to forward vector to decide that
            // so example: forward vector (0.5,0,-1), input (0.5,0,-1), same direction so moveforward

            auto modelRotation = glm::mat3(modelTransform);
            auto modelInverseRotation = glm::transpose(modelRotation);
            glm::vec3 characterInputDirection = modelInverseRotation * dir;

            auto normCharacterInputDirection = glm::normalize(characterInputDirection);
            if (glm::length(normCharacterInputDirection) > 0.00001f) {
                targetSpeed = normCharacterInputDirection.z;
                targetDirection = -normCharacterInputDirection.x;
            }
            else{
                targetSpeed = 0.0f;
                targetDirection = 0.0f;
            }

            inputXWorld = dir.x;
            inputZWorld = dir.z;
        }

        void setJumping()
        {
            isJumping = true;
            targetSpeed = 0.0f;
            targetDirection = 0.0f;
        }

        void setIdle()
        {
            targetSpeed = 0.0f;
            targetDirection = 1.0f;
            isJumping = false;

            directionVector = glm::vec3(0,0,0);
        }

        void update(glm::vec3 forward, glm::vec3 right, glm::quat characterRotation, glm::mat4 modelTransform)
        {
            this->forwardVector = forward;
            this->rightVector = right;
            this->characterRotation = characterRotation;
            this->modelTransform = modelTransform;

            movementSpeed = glm::mix(movementSpeed, targetSpeed, interpolationSpeed); // Smooth transition
            movementDirection = glm::mix(movementDirection, targetDirection, interpolationSpeed);

            movementSpeed = glm::mix(movementSpeed, targetSpeed, interpolationSpeed);
            movementDirection = glm::mix(movementDirection, targetDirection, interpolationSpeed);
        
            // If close to zero, force it to be zero
            constexpr float epsilon = 1e-6f;

            if (std::abs(movementSpeed - std::round(movementSpeed)) < epsilon) {
                movementSpeed = std::round(movementSpeed);
            }
        
            if (std::abs(movementDirection - std::round(movementDirection)) < epsilon) {
                movementDirection = std::round(movementDirection);
            }   
        }

        bool intersectRayWithYPlane(
        glm::vec3 rayOrigin, glm::vec3 rayDir, float planeY, glm::vec3& hitPoint)
        {
            const float denom = rayDir.y;
            const float EPS = 1e-6f;
            if (glm::abs(denom) < EPS) return false; // ray parallel to plane

            const float t = (planeY - rayOrigin.y) / denom;
            if (t < 0.0f) return false; // intersection behind the ray origin

            hitPoint = rayOrigin + t * rayDir;
            return true;
        }

    bool areInSameDirection(glm::vec3 v1, glm::vec3 v2, float angleDeg = 1.0f) {
        // Reject degenerate inputs
        if (glm::length2(v1) == 0.0f || glm::length2(v2) == 0.0f) return false;

        // Normalize to make the test scale-invariant
        v1 = glm::normalize(v1);
        v2 = glm::normalize(v2);  // normalize returns a new vector; it doesn't mutate. :contentReference[oaicite:2]{index=2}

        const float cosThresh = glm::cos(glm::radians(angleDeg));
        const float d = glm::dot(v1, v2);
        return d >= cosThresh;    // "same direction" within angleDeg
    }
        glm::quat faceMouseOnXZ(
        glm::vec3& playerPosition,
        float mouseX, float mouseY,
        glm::mat4& view,glm::mat4& proj);


    
    inline void register_bindings(LuaEngine& eng) {
        auto& L = eng.state();
        L.new_usertype<PlayerController>("PlayerController",
            "grounded", &PlayerController::grounded,
            "dodgeStart", &PlayerController::dodgeStart
        );
    }

    };
}