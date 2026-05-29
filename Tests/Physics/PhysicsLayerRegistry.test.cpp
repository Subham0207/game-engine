#include <gtest/gtest.h>

#include <string>

#include "Physics/PhysicsLayerRegistry.hpp"

namespace
{
    std::string makeUniqueLayerName(const char* prefix, const int line)
    {
        return std::string(prefix) + "_" + std::to_string(line);
    }
}

TEST(PhysicsLayerRegistry, shouldContainDefaultLayer)
{
    const auto& registry = Physics::PhysicsLayerRegistry::instance();
    EXPECT_TRUE(registry.hasLayer("Default"));
}

TEST(PhysicsLayerRegistry, shouldRegisterLayerAndResolveItsObjectId)
{
    auto& registry = Physics::PhysicsLayerRegistry::instance();
    const std::string layerName = makeUniqueLayerName("Layer", __LINE__);

    EXPECT_TRUE(registry.registerLayer(layerName));

    const auto resolvedLayer = registry.tryGetLayer(layerName);
    ASSERT_TRUE(resolvedLayer.has_value());
    EXPECT_EQ(resolvedLayer.value(), registry.getLayerOrDefault(layerName));
}

TEST(PhysicsLayerRegistry, shouldFallbackToDefaultLayerWhenNameIsUnknown)
{
    auto& registry = Physics::PhysicsLayerRegistry::instance();
    const auto defaultLayer = registry.getLayerOrDefault("Default");

    EXPECT_EQ(registry.getLayerOrDefault("UnknownLayerName"), defaultLayer);
}

TEST(PhysicsLayerRulebookRegistry, shouldUseRegisteredPairRuleSymmetrically)
{
    auto& layerRegistry = Physics::PhysicsLayerRegistry::instance();
    auto& rulebook = Physics::PhysicsLayerRulebookRegistry::instance();

    const std::string layerA = makeUniqueLayerName("NoPlayer", __LINE__);
    const std::string layerB = makeUniqueLayerName("NoEnemy", __LINE__);

    layerRegistry.registerLayer(layerA);
    layerRegistry.registerLayer(layerB);
    rulebook.registerRule(layerA, layerB, false);

    const auto objectA = layerRegistry.getLayerOrDefault(layerA);
    const auto objectB = layerRegistry.getLayerOrDefault(layerB);

    EXPECT_FALSE(rulebook.shouldCollide(objectA, objectB));
    EXPECT_FALSE(rulebook.shouldCollide(objectB, objectA));
}

