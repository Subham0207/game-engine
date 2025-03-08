#include <Controls/statemachine.hpp>
#include <Helpers/Shared.hpp>

Controls::AnimationStateMachine::AnimationStateMachine()
{
    Shared::readAnimation("E:/OpenGL/Models/Idle.fbx");
    Shared::readAnimation("E:/OpenGL/Models/Standard Walk.fbx");
    Shared::readAnimation("E:/OpenGL/Models/Running.fbx");
    Shared::readAnimation("E:/OpenGL/Models/Jumping.fbx");
}