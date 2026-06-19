#include <gtest/gtest.h>

#include <sstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "3DModel/model.hpp"
#include "Physics/PhysicsBodySettings.hpp"

namespace
{
    ProjectModals::Vertex makeVertex(const float x, const float y, const float z)
    {
        ProjectModals::Vertex v{};
        v.Position = glm::vec3(x, y, z);
        return v;
    }
}

TEST(ModelComputeLocalMeshHalfExtents, shouldUseMeshLocalBoundsInsteadOfAssumingUnitCube)
{
    // What/Why: Box collider sizing must follow model-local bounds so non-unit assets
    // still get correct rigid body fit after scaling in the level.
    Model model;

    Mesh mesh;
    mesh.vertices.push_back(makeVertex(-2.0f, -1.0f, -4.0f));
    mesh.vertices.push_back(makeVertex(6.0f, 3.0f, 2.0f));
    model.meshes.push_back(mesh);

    const glm::vec3 halfExtents = model.computeLocalMeshHalfExtents();

    EXPECT_FLOAT_EQ(halfExtents.x, 4.0f);
    EXPECT_FLOAT_EQ(halfExtents.y, 2.0f);
    EXPECT_FLOAT_EQ(halfExtents.z, 3.0f);
}

TEST(ModelComputeLocalMeshHalfExtents, shouldFallbackToIdentityHalfExtentsWhenNoMeshVerticesExist)
{
    // What/Why: Empty/placeholder models should keep a safe default collider size
    // instead of producing zero/negative extents that break physics body creation.
    Model model;

    const glm::vec3 halfExtents = model.computeLocalMeshHalfExtents();

    EXPECT_FLOAT_EQ(halfExtents.x, 1.0f);
    EXPECT_FLOAT_EQ(halfExtents.y, 1.0f);
    EXPECT_FLOAT_EQ(halfExtents.z, 1.0f);
}

TEST(ModelPhysicsBodySettingsSerialization, shouldRoundTripRigidCustomColliderSettings)
{
    // What/Why: Physics body settings are now authoritative model data and must
    // serialize deterministically to avoid losing collider setup across save/load.
    Physics::PhysicsBodySettings writeSettings;
    writeSettings.bodyType = Physics::BodyType::RigidBody;
    writeSettings.motionType = Physics::MotionType::Static;
    writeSettings.physicsLayer = "Gameplay";
    writeSettings.isSensor = true;
    writeSettings.rigidBodyData.colliderShape = Physics::ColliderShape::Custom;
    writeSettings.rigidBodyData.transformationOffset.position = glm::vec3(1.0f, 2.0f, 3.0f);
    writeSettings.rigidBodyData.transformationOffset.rotation = glm::vec3(10.0f, 20.0f, 30.0f);
    writeSettings.rigidBodyData.transformationOffset.scale = glm::vec3(2.0f, 3.0f, 4.0f);
    writeSettings.rigidBodyData.mass = 42.0f;
    writeSettings.rigidBodyData.overrideMass = true;
    writeSettings.rigidBodyData.centerOfMassOffset = glm::vec3(0.5f, -0.25f, 1.25f);
    writeSettings.rigidBodyData.friction = 0.65f;
    writeSettings.rigidBodyData.restitution = 0.3f;
    writeSettings.rigidBodyData.linearDamping = 0.12f;
    writeSettings.rigidBodyData.angularDamping = 0.34f;

    Physics::CustomColliderShapeData cooked;
    cooked.vertices.emplace_back(0.0f, 0.0f, 0.0f);
    cooked.vertices.emplace_back(1.0f, 0.0f, 0.0f);
    cooked.vertices.emplace_back(0.0f, 1.0f, 0.0f);
    cooked.indices = {0, 1, 2};
    writeSettings.rigidBodyData.customColliderShapeData = cooked;

    std::stringstream buffer;
    {
        boost::archive::text_oarchive oa(buffer);
        oa << writeSettings;
    }

    Physics::PhysicsBodySettings readSettings;
    {
        boost::archive::text_iarchive ia(buffer);
        ia >> readSettings;
    }

    EXPECT_EQ(readSettings.bodyType, Physics::BodyType::RigidBody);
    EXPECT_EQ(readSettings.motionType, Physics::MotionType::Static);
    EXPECT_EQ(readSettings.physicsLayer, "Gameplay");
    EXPECT_TRUE(readSettings.isSensor);
    EXPECT_EQ(readSettings.rigidBodyData.colliderShape, Physics::ColliderShape::Custom);
    EXPECT_FLOAT_EQ(readSettings.rigidBodyData.mass, 42.0f);
    EXPECT_TRUE(readSettings.rigidBodyData.overrideMass);
    EXPECT_FLOAT_EQ(readSettings.rigidBodyData.centerOfMassOffset.x, 0.5f);
    EXPECT_FLOAT_EQ(readSettings.rigidBodyData.centerOfMassOffset.y, -0.25f);
    EXPECT_FLOAT_EQ(readSettings.rigidBodyData.centerOfMassOffset.z, 1.25f);
    EXPECT_FLOAT_EQ(readSettings.rigidBodyData.friction, 0.65f);
    EXPECT_FLOAT_EQ(readSettings.rigidBodyData.restitution, 0.3f);
    EXPECT_FLOAT_EQ(readSettings.rigidBodyData.linearDamping, 0.12f);
    EXPECT_FLOAT_EQ(readSettings.rigidBodyData.angularDamping, 0.34f);
    ASSERT_TRUE(readSettings.rigidBodyData.customColliderShapeData.has_value());

    const auto& readCooked = readSettings.rigidBodyData.customColliderShapeData.value();
    ASSERT_EQ(readCooked.vertices.size(), 3u);
    ASSERT_EQ(readCooked.indices.size(), 3u);
    EXPECT_EQ(readCooked.indices[0], 0u);
    EXPECT_EQ(readCooked.indices[1], 1u);
    EXPECT_EQ(readCooked.indices[2], 2u);
}

TEST(ModelGameplayTags, shouldQueryConfiguredTags)
{
    Model model;
    model.setGameplayTags({"Player", "Damageable", "Hero"});

    EXPECT_TRUE(model.hasGameplayTag("Player"));
    EXPECT_TRUE(model.hasGameplayTag("Damageable"));
    EXPECT_FALSE(model.hasGameplayTag("Enemy"));
}

TEST(ModelGameplayTags, shouldReplacePreviousTagsWhenTagsAreReset)
{
    Model model;
    model.setGameplayTags({"Player", "Damageable"});
    model.setGameplayTags({"Enemy"});

    EXPECT_FALSE(model.hasGameplayTag("Player"));
    EXPECT_FALSE(model.hasGameplayTag("Damageable"));
    EXPECT_TRUE(model.hasGameplayTag("Enemy"));
    EXPECT_EQ(model.GetGameplayTags().size(), 1u);
}

TEST(ModelPhysicsBodySettingsSerialization, shouldRoundTripConvexHullColliderSelection)
{
    Physics::PhysicsBodySettings writeSettings;
    writeSettings.bodyType = Physics::BodyType::RigidBody;
    writeSettings.motionType = Physics::MotionType::Dynamic;
    writeSettings.rigidBodyData.colliderShape = Physics::ColliderShape::ConvexHull;

    Physics::CustomColliderShapeData cooked;
    cooked.vertices.emplace_back(0.0f, 0.0f, 0.0f);
    cooked.vertices.emplace_back(1.0f, 0.0f, 0.0f);
    cooked.vertices.emplace_back(0.0f, 1.0f, 0.0f);
    cooked.vertices.emplace_back(0.0f, 0.0f, 1.0f);
    cooked.indices = {0, 1, 2, 0, 2, 3};
    writeSettings.rigidBodyData.customColliderShapeData = cooked;

    std::stringstream buffer;
    {
        boost::archive::text_oarchive oa(buffer);
        oa << writeSettings;
    }

    Physics::PhysicsBodySettings readSettings;
    {
        boost::archive::text_iarchive ia(buffer);
        ia >> readSettings;
    }

    EXPECT_EQ(readSettings.motionType, Physics::MotionType::Dynamic);
    EXPECT_EQ(readSettings.rigidBodyData.colliderShape, Physics::ColliderShape::ConvexHull);
    ASSERT_TRUE(readSettings.rigidBodyData.customColliderShapeData.has_value());
    EXPECT_EQ(readSettings.rigidBodyData.customColliderShapeData->vertices.size(), 4u);
}

TEST(ModelClassIdSerialization, shouldRoundTripClassIdForVersion1Archive)
{
    Model writeModel;
    writeModel.classId = "CustomGameplayModel";

    std::stringstream buffer;
    {
        boost::archive::text_oarchive oa(buffer);
        oa << writeModel;
    }

    Model readModel;
    {
        boost::archive::text_iarchive ia(buffer);
        ia >> readModel;
    }

    EXPECT_EQ(readModel.classId, "CustomGameplayModel");
}
