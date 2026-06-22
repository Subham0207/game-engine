#include <gtest/gtest.h>

#include "3DModel/Animation/Animator.hpp"
#include <glm/gtc/matrix_transform.hpp>

TEST(AnimatorRegionEvents, shouldCollectEnterExitAcrossAnimationBoundaries)
{
    const std::vector<AnimationRegion> regions = {
        {"Attack", 0.2f, 0.4f}
    };

    const auto enterEvents = Animator::CollectRegionBoundaryEvents(regions, 0.1f, 0.3f, 1.0f, false);
    ASSERT_EQ(enterEvents.size(), 1u);
    EXPECT_EQ(enterEvents[0].type, AnimEventType::RegionEnter);
    EXPECT_EQ(enterEvents[0].regionName, "Attack");

    const auto exitEvents = Animator::CollectRegionBoundaryEvents(regions, 0.3f, 0.5f, 1.0f, false);
    ASSERT_EQ(exitEvents.size(), 1u);
    EXPECT_EQ(exitEvents[0].type, AnimEventType::RegionExit);
    EXPECT_EQ(exitEvents[0].regionName, "Attack");

    const std::vector<AnimationRegion> wrapRegions = {
        {"Recover", 0.8f, 0.95f},
        {"LoopStart", 0.05f, 0.15f}
    };
    const auto wrapEvents = Animator::CollectRegionBoundaryEvents(wrapRegions, 0.9f, 0.12f, 1.0f, true);
    ASSERT_EQ(wrapEvents.size(), 2u);
    EXPECT_EQ(wrapEvents[0].type, AnimEventType::RegionExit);
    EXPECT_EQ(wrapEvents[0].regionName, "Recover");
    EXPECT_EQ(wrapEvents[1].type, AnimEventType::RegionEnter);
    EXPECT_EQ(wrapEvents[1].regionName, "LoopStart");
}

TEST(AnimatorRegionEvents, shouldGateBlendspaceCandidateSelectionByPoseThreshold)
{
    const std::vector<glm::mat4> finalPose = {glm::mat4(1.0f)};
    const std::vector<std::vector<glm::mat4>> passingCandidates = {
        {glm::mat4(1.0f)},
        {glm::translate(glm::mat4(1.0f), glm::vec3(0.3f, 0.0f, 0.0f))}
    };

    const auto passingSelection = Animator::SelectBestPoseMatchIndex(passingCandidates, finalPose, 0.10f);
    ASSERT_TRUE(passingSelection.has_value());
    EXPECT_EQ(*passingSelection, 0u);

    const std::vector<std::vector<glm::mat4>> blockedCandidates = {
        {glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f))},
        {glm::translate(glm::mat4(1.0f), glm::vec3(0.8f, 0.0f, 0.0f))}
    };
    const auto blockedSelection = Animator::SelectBestPoseMatchIndex(blockedCandidates, finalPose, 0.10f);
    EXPECT_FALSE(blockedSelection.has_value());
}
