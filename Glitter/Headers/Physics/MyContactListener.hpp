#pragma once
#include <PhysicsSystem.hpp>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <utility>
#include <unordered_map>
#include <unordered_set>

class MyContactListener : public JPH::CharacterContactListener
{
public:
    void beginFrame()
    {
        has_landed_this_frame = false;
        character_contacts_this_frame.clear();
        character_contact_points_this_frame.clear();
    }

    [[nodiscard]] bool hasCharacterContacts() const
    {
        return !character_contacts_this_frame.empty();
    }

    [[nodiscard]] const std::unordered_set<uint32_t>& getCharacterContactIds() const
    {
        return character_contacts_this_frame;
    }

    [[nodiscard]] const std::unordered_map<uint32_t, std::vector<glm::vec3>>& getCharacterContactPoints() const
    {
        return character_contact_points_this_frame;
    }

    void setCharacterCollisionRuleEvaluator(std::function<bool(uint32_t, uint32_t)> evaluator)
    {
        character_collision_rule_evaluator = std::move(evaluator);
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

    bool OnCharacterContactValidate(const JPH::CharacterVirtual *character,
                                    const JPH::CharacterVirtual *otherCharacter,
                                    const JPH::SubShapeID &subShape) override
    {
        if (!character_collision_rule_evaluator)
        {
            return true;
        }

        return character_collision_rule_evaluator(
            character->GetID().GetValue(),
            otherCharacter->GetID().GetValue());
    }

    void OnCharacterContactAdded(const JPH::CharacterVirtual *character,
                                 const JPH::CharacterVirtual *otherCharacter,
                                 const JPH::SubShapeID &subShape,
                                 JPH::RVec3Arg pos,
                                 JPH::Vec3Arg normal,
                                 JPH::CharacterContactSettings &ioSettings) override
    {
        character_contacts_this_frame.insert(otherCharacter->GetID().GetValue());
        character_contact_points_this_frame[otherCharacter->GetID().GetValue()].emplace_back(
            static_cast<float>(pos.GetX()),
            static_cast<float>(pos.GetY()),
            static_cast<float>(pos.GetZ()));
    }

    void OnCharacterContactPersisted(const JPH::CharacterVirtual *character,
                                     const JPH::CharacterVirtual *otherCharacter,
                                     const JPH::SubShapeID &subShape,
                                     JPH::RVec3Arg pos,
                                     JPH::Vec3Arg normal,
                                     JPH::CharacterContactSettings &ioSettings) override
    {
        character_contacts_this_frame.insert(otherCharacter->GetID().GetValue());
        character_contact_points_this_frame[otherCharacter->GetID().GetValue()].emplace_back(
            static_cast<float>(pos.GetX()),
            static_cast<float>(pos.GetY()),
            static_cast<float>(pos.GetZ()));
    }

public:
    bool has_landed_this_frame = false;

private:
    std::unordered_set<uint32_t> character_contacts_this_frame;
    std::unordered_map<uint32_t, std::vector<glm::vec3>> character_contact_points_this_frame;
    std::function<bool(uint32_t, uint32_t)> character_collision_rule_evaluator;
};