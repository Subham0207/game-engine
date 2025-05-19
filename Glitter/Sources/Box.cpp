#include "Physics/Box.hpp"
#include <glm/glm.hpp>
#include <EngineState.hpp>
#include <Jolt/Physics/PhysicsSystem.h>

Physics::Box::Box(
    PhysicsSystemWrapper* physics,
    bool isDynamic,
    bool shouldAddToLevel,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
) : PhysicsObject(
        physics,
        "E:/OpenGL/Glitter/EngineAssets/cube.fbx",
        isDynamic,
        shouldAddToLevel,
        position,
        rotation,
        scale
    )
{
}