#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Physics
{
    class PhysicsLayerRegistry
    {
    public:
        static PhysicsLayerRegistry& instance();

        // Register one layer at a time to keep runtime registration explicit.
        bool registerLayer(const std::string& layerName);

        [[nodiscard]] bool hasLayer(const std::string& layerName) const;
        [[nodiscard]] std::optional<JPH::ObjectLayer> tryGetLayer(const std::string& layerName) const;
        [[nodiscard]] JPH::ObjectLayer getLayerOrDefault(const std::string& layerName) const;
        [[nodiscard]] const std::vector<std::string>& getLayerNames() const;

    private:
        PhysicsLayerRegistry();

        std::vector<std::string> layerNames;
        std::unordered_map<std::string, JPH::ObjectLayer> layerLookup;
    };

    class PhysicsLayerRulebookRegistry
    {
    public:
        static PhysicsLayerRulebookRegistry& instance();

        // Register one interaction rule at a time.
        void registerRule(const std::string& layerA, const std::string& layerB, bool shouldCollide);
        [[nodiscard]] bool shouldCollide(JPH::ObjectLayer layerA, JPH::ObjectLayer layerB) const;

    private:
        static uint32_t makeRuleKey(JPH::ObjectLayer layerA, JPH::ObjectLayer layerB);

        std::unordered_map<uint32_t, bool> rules;
    };
}

#define PHYSICS_LAYER_CONCAT_IMPL(x, y) x##y
#define PHYSICS_LAYER_CONCAT(x, y) PHYSICS_LAYER_CONCAT_IMPL(x, y)

#define REGISTER_PHYSICS_LAYER_IMPL(LayerNameLiteral, UniqueId) \
namespace { \
    struct PHYSICS_LAYER_CONCAT(PhysicsLayerRegistrar_, UniqueId) { \
        PHYSICS_LAYER_CONCAT(PhysicsLayerRegistrar_, UniqueId)() { \
            Physics::PhysicsLayerRegistry::instance().registerLayer(LayerNameLiteral); \
        } \
    } PHYSICS_LAYER_CONCAT(gPhysicsLayerRegistrar_, UniqueId); \
}

#define REGISTER_PHYSICS_LAYER(LayerNameLiteral) REGISTER_PHYSICS_LAYER_IMPL(LayerNameLiteral, __COUNTER__)

#define REGISTER_PHYSICS_LAYER_RULE_IMPL(LayerA, LayerB, ShouldCollide, UniqueId) \
namespace { \
    struct PHYSICS_LAYER_CONCAT(PhysicsLayerRuleRegistrar_, UniqueId) { \
        PHYSICS_LAYER_CONCAT(PhysicsLayerRuleRegistrar_, UniqueId)() { \
            Physics::PhysicsLayerRulebookRegistry::instance().registerRule(LayerA, LayerB, ShouldCollide); \
        } \
    } PHYSICS_LAYER_CONCAT(gPhysicsLayerRuleRegistrar_, UniqueId); \
}

#define REGISTER_PHYSICS_LAYER_RULE(LayerA, LayerB, ShouldCollide) REGISTER_PHYSICS_LAYER_RULE_IMPL(LayerA, LayerB, ShouldCollide, __COUNTER__)




