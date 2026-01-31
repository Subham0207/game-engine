#include <WarriorCharacter.hpp>
#include <EngineState.hpp>
#include <Controls/Input.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

REGISTER_TYPE(WarriorCharacter, "WarriorCharacter", CharacterFactory)

void WarriorCharacter::onStart()
{
    inputHandler = InputHandler::currentInputHandler;

    EngineState::state->bus.subscribe<MouseMoveEvent>([&](const MouseMoveEvent& e)
    {        
        springArm.moveArm(e);
    });
}

void WarriorCharacter::onDestroy()
{
}

void WarriorCharacter::onTick()
{
    handleInputs();
    handleSpringArmAndCamera();

    auto sm = getDerivedStateMachine();
    auto blendspace = sm->getLocomotionBlend();
    grounded = capsuleCollider->grounded;
    sm->setGrounded(grounded);
    sm->setDodgeStart(dodgeStart);

    auto cameraFront = this->camera->getFront();
    auto cameraRight = this->camera->getRight();
    glm::vec3 forwardXZ = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    glm::vec3 rightXZ = glm::normalize(glm::vec3(cameraRight.x, 0.0f, cameraRight.z));

    auto movementOffset = forwardXZ * inputZWorld
    - rightXZ   * inputXWorld;

    assert(!glm::any(glm::isnan(movementOffset)) && "movementOffset contains NaN values!");
    set_movement_offset(movementOffset);

    glm::quat desiredRot{};
    auto dir = glm::vec3(inputXWorld, 0.0f,inputZWorld);
    blendspace->setScrubberLocation(glm::vec2(inputXWorld, inputZWorld));
    if(glm::length(dir) > 0.00001f) // Essentially means W_PRESSED then face camera direction
    {
        if(isAiming)
        {
            desiredRot =  glm::angleAxis(springArm.getTheta(), glm::vec3(0,1,0));
            lastPlayerYaw = springArm.getTheta();
        }
        else
        {
            blendspace->setScrubberLocation(glm::vec2(0.0f, 1.0f)); // to use forward walking animation while turning the character.
            // calculate desired rotation based on camera view
            auto cameraFront = this->camera->getFront();
            auto cameraRight = this->camera->getRight();
            glm::vec3 forwardXZ = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
            glm::vec3 rightXZ = glm::normalize(glm::vec3(cameraRight.x, 0.0f, cameraRight.z));
            glm::vec3 moveDir = forwardXZ * inputZWorld
            - rightXZ   * inputXWorld;
            float targetYaw = std::atan2(moveDir.x, moveDir.z);
            float yaw = smoothAngle(lastPlayerYaw, targetYaw, interpolationSpeed);
            lastPlayerYaw= yaw;
            desiredRot = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
        }
    }
    else
    {
        desiredRot =  glm::angleAxis(lastPlayerYaw, glm::vec3(0,1,0));
    }

    set_rotation_offset(desiredRot);
}

void WarriorCharacter::handleInputs()
{
    glm::vec3 directionVector{};
    if (inputHandler->isKeyPressed(GLFW_KEY_W))
        directionVector += glm::vec3(0,0,1);
    if (inputHandler->isKeyPressed(GLFW_KEY_A))
        directionVector += glm::vec3(1,0,0);
    if (inputHandler->isKeyPressed(GLFW_KEY_D))
        directionVector += glm::vec3(-1,0,0);
    if (inputHandler->isKeyPressed(GLFW_KEY_S))
        directionVector += glm::vec3(0,0,-1);
    if (inputHandler->isKeyPressed(GLFW_KEY_SPACE) && grounded)
        isJumping = true;
    if (inputHandler->isKeyPressed(GLFW_KEY_E))
    {
        dodgeStart = true;
        directionVector = glm::vec3(0.0f);
    }

    isAiming = inputHandler->rightClickPressed;

    if (glm::length(directionVector) > 0.00001f) {
        setMovement(glm::normalize(directionVector));
    }
    else
    {
        setMovement(glm::vec3(0.0f,0.0f,0.0f));
    }
}

void WarriorCharacter::setMovement(glm::vec3 dir)
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

void WarriorCharacter::handleSpringArmAndCamera()
{
    glm::vec3 pivotPosition = model->GetPosition();
    assert(!glm::any(glm::isnan(pivotPosition)) && "Model position is NaN before update!");
    pivotPosition.y+=5.0f;
    springArm.setPivotPos(pivotPosition);

    auto camerapos = springArm.getEndPosition();
    this->camera->cameraPos = camerapos;
    this->camera->lookAt(pivotPosition);

    springArm.onTick();
}
