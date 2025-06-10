#pragma once
#include <glm/glm.hpp>
#include <iostream>

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

        void setMovement(float speed, float direction, glm::vec3 dir)
        {
            targetSpeed = speed;         // Set target speed (Idle: 0, Walk: 1, Run: 2)
            targetDirection = direction; // Set target direction (Normalized X,Y)
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

        void update()
        {
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
    };
}