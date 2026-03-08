#include <WarriorAICharacter.hpp>

REGISTER_TYPE(WarriorAICharacter, "WarriorAICharacter", CharacterFactory)

void WarriorAICharacter::onStart(){
    aiController = std::dynamic_pointer_cast<WarriorAIController>(this->controller);
    sm = std::static_pointer_cast<WarriorStatemachine>(this->animStateMachine);
}

void WarriorAICharacter::onDestroy(){
    
}

void WarriorAICharacter::onTick(){
    aiController->setCurrentCharacterPosition(GetPosition());
    
    auto blendspace = sm->getLocomotionBlend();
    auto grounded = capsuleCollider->grounded;
    sm->setGrounded(grounded);
    aiController->setGrounded(grounded);
    sm->setDodgeStart(false);

    auto moveDir = aiController->getTargetWorldDirection();
    set_movement_offset(moveDir);

    glm::quat desiredRot{};
    if(glm::length(moveDir) > 0.00001f)
    {
        float targetYaw = std::atan2(moveDir.x, moveDir.z);
        float yaw = smoothAngle(lastPlayerYaw, targetYaw, interpolationSpeed);
        lastPlayerYaw = yaw;
        desiredRot = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
        blendspace->setScrubberLocation(glm::vec2(0.0f, 1.0f)); // use forward moving animation
    }
    else
    {
        desiredRot =  glm::angleAxis(lastPlayerYaw, glm::vec3(0,1,0));
        blendspace->setScrubberLocation(glm::vec2(0.0f, 0.0f)); // play idle animation
    }

    set_rotation_offset(desiredRot);
}