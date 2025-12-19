#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/quaternion.hpp>
#include <LuaEngine/LuaEngine.hpp>
#include <Helpers/shader.hpp>

#include "LuaEngine/LuaRegistry.hpp"
#include "LuaEngine/LuaClassBuilder.hpp"
#include "LuaEngine/RegisterBinding.hpp"
enum CameraType;

namespace Controls
{
    class PlayerController: public RegisterBinding
    {        
    public:
        PlayerController(std::string filename);

        float movementSpeed = 0.0f;           // Current speed (blended)
        float targetSpeed = 0.0f;             // Target speed (where we want to go)
        float movementDirection = 0.0f;   // Current direction (blended)
        float targetDirection= 0.0f;     // Target direction (where we want to go)
        bool isJumping;
        bool isAiming;
        bool isForwardPressed = false;
        bool grounded;
        bool dodgeStart;
        float interpolationSpeed;      // Controls how fast blending happens (0.1 = smooth, 1.0 = instant)
        Shader* rayCastshader =  new Shader("./Shaders/rayCast.vert", "./Shaders/rayCast.frag");

        glm::vec3 lookDirection;

        glm::vec3 directionVector;

        glm::mat3 characterRotation;
        glm::vec3 characterPosition;

        float inputXWorld;
        float inputZWorld;
        float inputVectorLength;

        std::string filename;

        CameraType cameraType;

        void setMovement(glm::vec3 dir);

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

        void update(glm::mat3 characterRotation, glm::vec3 characterPosition)
        {

            this->characterPosition = characterPosition;
            this->characterRotation = characterRotation;

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


    
    inline static void register_bindings(LuaEngine& eng) {
        auto& L = eng.state();
        L.new_usertype<PlayerController>("PlayerController",
            "grounded", &PlayerController::grounded,
            "dodgeStart", &PlayerController::dodgeStart
        );
    }

    static void RegisterEngineAPI(LuaRegistry& reg)
    {
        // PlayerController
        reg.beginClass<PlayerController>("PlayerController")
            .field("movementSpeed", &PlayerController::movementSpeed, "number")
            .method("setMovement", &PlayerController::setMovement,
                    "fun(self: PlayerController, dir: Vec3)", "Sets desired movement direction")
            .property_readonly("aiming", &PlayerController::isAiming,
                    "boolean", "True if aiming");

        // Globals (stub-side)
        reg.addGlobal("Engine", "Engine");
        reg.addGlobal("Scene", "Scene");
    }

    };
}