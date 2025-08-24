#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Controls
{
    class PlayerController
    {        
    public:
        PlayerController()
            : movementSpeed(0.0f), targetSpeed(0.0f), movementDirection(0.0f), targetDirection(0.0f),
              isJumping(false), interpolationSpeed(0.1f), directionVector(0.0f,0.0f,0.0f)
        {}

        float movementSpeed;           // Current speed (blended)
        float targetSpeed;             // Target speed (where we want to go)
        float movementDirection;   // Current direction (blended)
        float targetDirection;     // Target direction (where we want to go)
        bool isJumping;
        float interpolationSpeed;      // Controls how fast blending happens (0.1 = smooth, 1.0 = instant)

        glm::vec3 directionVector;

        glm::vec3 forwardVector;
        glm::vec3 rightVector;
        glm::quat characterRotation;
        glm::mat4 modelTransform;

        void setMovement(float x, float z, glm::vec3 dir)
        {
            // The input here is actually normalized vector in world space from character position.
            // so input alone should not decide which animation to play
            // compare  input to forward vector to decide that
            // so example: forward vector (0.5,0,-1), input (0.5,0,-1), same direction so moveforward

            auto modelInverseTransform =  glm::inverse(modelTransform);
            glm::vec4 worldInputDirectionHomogeneous(dir, 0.0f);
            glm::vec4 characterInputDirection = glm::normalize(modelInverseTransform * worldInputDirectionHomogeneous);
            characterInputDirection.x += 1;

            targetSpeed = characterInputDirection.z;

            targetDirection = characterInputDirection.x;


            isJumping = false;

            directionVector = dir;
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
    };
}