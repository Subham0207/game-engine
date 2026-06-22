#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "3DModel/Animation/Animation.hpp"
#include "EngineState.hpp"

namespace
{
    struct EngineStateScope
    {
        EngineState state;

        EngineStateScope()
        {
            EngineState::state = &state;
        }

        ~EngineStateScope()
        {
            EngineState::state = nullptr;
        }
    };
}

TEST(AnimationMetaRegions, shouldLoadRegionsFromMetaJsonSkippingInvalidEntries)
{
    EngineStateScope scope;

    const auto workspace = std::filesystem::current_path() / "AnimationMetaTestWorkspace";
    std::filesystem::remove_all(workspace);
    std::filesystem::create_directories(workspace);

    const std::string guid = "animation-load-guid";
    const auto contentFile = workspace / "walk.animation";
    const auto metaFile = workspace / (guid + ".meta.json");

    std::ofstream(contentFile.string()).close();

    boost::property_tree::ptree meta;
    meta.put("guid", guid);
    meta.put("type", "animation");
    meta.put("version", "1");
    meta.put("content.relative_path", contentFile.filename().string());

    boost::property_tree::ptree regions;
    {
        boost::property_tree::ptree region;
        region.put("name", "Idle");
        region.put("startTime", 0.1f);
        region.put("endTime", 0.4f);
        regions.push_back(std::make_pair("", region));
    }
    {
        boost::property_tree::ptree region;
        region.put("startTime", 0.5f);
        region.put("endTime", 1.0f);
        regions.push_back(std::make_pair("", region));
    }
    meta.put_child("regions", regions);
    boost::property_tree::write_json(metaFile.string(), meta);

    scope.state.engineRegistry->renderableSaveFileMap[guid] = contentFile.string();
    scope.state.engineRegistry->animationsFileMap[guid] = contentFile.string();

    Animation animation;
    animation.setAssetId(guid);
    animation.loadRegionsFromMeta();

    ASSERT_EQ(animation.regions.size(), 1u);
    EXPECT_EQ(animation.regions[0].name, "Idle");
    EXPECT_FLOAT_EQ(animation.regions[0].startTime, 0.1f);
    EXPECT_FLOAT_EQ(animation.regions[0].endTime, 0.4f);

    std::filesystem::remove_all(workspace);
}

TEST(AnimationMetaRegions, shouldSaveRegionsToMetaJsonAndPreserveExistingKeys)
{
    EngineStateScope scope;

    const auto workspace = std::filesystem::current_path() / "AnimationMetaTestWorkspace";
    std::filesystem::remove_all(workspace);
    std::filesystem::create_directories(workspace);

    const std::string guid = "animation-save-guid";
    const auto contentFile = workspace / "run.animation";
    const auto metaFile = workspace / (guid + ".meta.json");

    std::ofstream(contentFile.string()).close();

    boost::property_tree::ptree meta;
    meta.put("guid", guid);
    meta.put("type", "animation");
    meta.put("version", "5");
    meta.put("content.relative_path", contentFile.filename().string());
    meta.put("custom.keep_me", "yes");
    boost::property_tree::write_json(metaFile.string(), meta);

    scope.state.engineRegistry->renderableSaveFileMap[guid] = contentFile.string();
    scope.state.engineRegistry->animationsFileMap[guid] = contentFile.string();

    Animation animation;
    animation.setAssetId(guid);
    animation.regions = {
        {"Intro", 0.0f, 0.8f},
        {"Loop", 0.5f, 1.5f}
    };

    ASSERT_TRUE(animation.saveRegionsToMeta());

    boost::property_tree::ptree savedMeta;
    boost::property_tree::read_json(metaFile.string(), savedMeta);

    EXPECT_EQ(savedMeta.get<std::string>("guid"), guid);
    EXPECT_EQ(savedMeta.get<std::string>("type"), "animation");
    EXPECT_EQ(savedMeta.get<std::string>("version"), "5");
    EXPECT_EQ(savedMeta.get<std::string>("content.relative_path"), contentFile.filename().string());
    EXPECT_EQ(savedMeta.get<std::string>("custom.keep_me"), "yes");

    const auto& regions = savedMeta.get_child("regions");
    ASSERT_EQ(regions.size(), 2u);
    auto it = regions.begin();
    EXPECT_EQ(it->second.get<std::string>("name"), "Intro");
    EXPECT_FLOAT_EQ(it->second.get<float>("startTime"), 0.0f);
    EXPECT_FLOAT_EQ(it->second.get<float>("endTime"), 0.8f);
    ++it;
    EXPECT_EQ(it->second.get<std::string>("name"), "Loop");
    EXPECT_FLOAT_EQ(it->second.get<float>("startTime"), 0.5f);
    EXPECT_FLOAT_EQ(it->second.get<float>("endTime"), 1.5f);

    std::filesystem::remove_all(workspace);
}
