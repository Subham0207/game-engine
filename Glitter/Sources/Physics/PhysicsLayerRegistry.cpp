#include "Physics/PhysicsLayerRegistry.hpp"
#include <algorithm>
namespace
{
    constexpr JPH::ObjectLayer kDefaultLayer = static_cast<JPH::ObjectLayer>(0);
}
Physics::PhysicsLayerRegistry& Physics::PhysicsLayerRegistry::instance()
{
    static PhysicsLayerRegistry registry;
    return registry;
}
Physics::PhysicsLayerRegistry::PhysicsLayerRegistry()
{
    registerLayer("Default");
}
bool Physics::PhysicsLayerRegistry::registerLayer(const std::string& layerName)
{
    if (layerName.empty() || layerLookup.find(layerName) != layerLookup.end())
    {
        return false;
    }
    const auto layerId = static_cast<JPH::ObjectLayer>(layerNames.size());
    layerNames.push_back(layerName);
    layerLookup[layerName] = layerId;
    return true;
}
bool Physics::PhysicsLayerRegistry::hasLayer(const std::string& layerName) const
{
    return layerLookup.find(layerName) != layerLookup.end();
}
std::optional<JPH::ObjectLayer> Physics::PhysicsLayerRegistry::tryGetLayer(const std::string& layerName) const
{
    const auto found = layerLookup.find(layerName);
    if (found == layerLookup.end())
    {
        return std::nullopt;
    }
    return found->second;
}
JPH::ObjectLayer Physics::PhysicsLayerRegistry::getLayerOrDefault(const std::string& layerName) const
{
    const auto found = layerLookup.find(layerName);
    if (found != layerLookup.end())
    {
        return found->second;
    }
    return kDefaultLayer;
}
const std::vector<std::string>& Physics::PhysicsLayerRegistry::getLayerNames() const
{
    return layerNames;
}
Physics::PhysicsLayerRulebookRegistry& Physics::PhysicsLayerRulebookRegistry::instance()
{
    static PhysicsLayerRulebookRegistry rulebook;
    return rulebook;
}
void Physics::PhysicsLayerRulebookRegistry::registerRule(
    const std::string& layerA,
    const std::string& layerB,
    const bool shouldCollide)
{
    auto& layerRegistry = PhysicsLayerRegistry::instance();
    if (!layerRegistry.hasLayer(layerA))
    {
        layerRegistry.registerLayer(layerA);
    }
    if (!layerRegistry.hasLayer(layerB))
    {
        layerRegistry.registerLayer(layerB);
    }
    const auto lhs = layerRegistry.getLayerOrDefault(layerA);
    const auto rhs = layerRegistry.getLayerOrDefault(layerB);
    rules[makeRuleKey(lhs, rhs)] = shouldCollide;
}
bool Physics::PhysicsLayerRulebookRegistry::shouldCollide(JPH::ObjectLayer layerA, JPH::ObjectLayer layerB) const
{
    const auto found = rules.find(makeRuleKey(layerA, layerB));
    if (found != rules.end())
    {
        return found->second;
    }
    return true;
}
uint32_t Physics::PhysicsLayerRulebookRegistry::makeRuleKey(JPH::ObjectLayer layerA, JPH::ObjectLayer layerB)
{
    const auto lhs = static_cast<uint16_t>(std::min(layerA, layerB));
    const auto rhs = static_cast<uint16_t>(std::max(layerA, layerB));
    return (static_cast<uint32_t>(lhs) << 16u) | static_cast<uint32_t>(rhs);
}
