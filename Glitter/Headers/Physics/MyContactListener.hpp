#pragma once
#include <PhysicsSystem.hpp>
#include <Jolt/Physics/Character/CharacterVirtual.h>

class MyContactListener : public JPH::CharacterContactListener
{
    // Called for every *new* manifold. Handy for footstep SFX, landing, etc.
    void OnContactAdded(const JPH::CharacterVirtual *character,
                        const JPH::BodyID &otherBody,
                        const JPH::SubShapeID &subShape,
                        JPH::RVec3Arg pos,
                        JPH::Vec3Arg normal,
                        JPH::CharacterContactSettings &ioSettings) override
    {
        if (character->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround)
            has_landed_this_frame = true;
    }

    
    public:
      bool has_landed_this_frame = false;
      ~MyContactListener(){};
};