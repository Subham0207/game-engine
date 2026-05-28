#pragma once
#include <PhysicsSystem.hpp>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <cstdint>
#include <unordered_set>

class MyContactListener : public JPH::CharacterContactListener
{
public:
    void beginFrame()
    {
        has_landed_this_frame = false;
        character_contacts_this_frame.clear();
    }

    [[nodiscard]] bool hasCharacterContacts() const
    {
        return !character_contacts_this_frame.empty();
    }

    [[nodiscard]] const std::unordered_set<uint32_t>& getCharacterContactIds() const
    {
        return character_contacts_this_frame;
    }

    ~MyContactListener() override = default;

private:
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

    void OnCharacterContactAdded(const JPH::CharacterVirtual *character,
                                 const JPH::CharacterVirtual *otherCharacter,
                                 const JPH::SubShapeID &subShape,
                                 JPH::RVec3Arg pos,
                                 JPH::Vec3Arg normal,
                                 JPH::CharacterContactSettings &ioSettings) override
    {
        character_contacts_this_frame.insert(otherCharacter->GetID().GetValue());
    }

    void OnCharacterContactPersisted(const JPH::CharacterVirtual *character,
                                     const JPH::CharacterVirtual *otherCharacter,
                                     const JPH::SubShapeID &subShape,
                                     JPH::RVec3Arg pos,
                                     JPH::Vec3Arg normal,
                                     JPH::CharacterContactSettings &ioSettings) override
    {
        character_contacts_this_frame.insert(otherCharacter->GetID().GetValue());
    }

public:
    bool has_landed_this_frame = false;

private:
    std::unordered_set<uint32_t> character_contacts_this_frame;
};