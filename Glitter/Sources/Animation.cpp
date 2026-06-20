#include <3DModel/Animation/Animation.hpp>
#include <iostream>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "EngineState.hpp"

Animation* Animation::loadAnimation(std::string guid)
{
    if (loadedAnimations.find(guid) != loadedAnimations.end())
        return loadedAnimations[guid];

    const auto filesMap = getEngineRegistryFilesMap();
    const auto found = filesMap.find(guid);
    if (found == filesMap.end())
        return nullptr;

    auto parentPath = fs::path(found->second).parent_path();
    auto animation = new Animation();
    animation->load(parentPath, guid);
    animation->loadRegionsFromMeta();
    loadedAnimations[guid] = animation;

    return animation;
}

void Animation::loadRegionsFromMeta()
{
    regions.clear();

    auto filesMap = getEngineRegistryFilesMap();
    const auto found = filesMap.find(getAssetId());
    if (found == filesMap.end())
        return;

    const fs::path metaFile = fs::path(found->second).parent_path() / (getAssetId() + ".meta.json");
    if (!fs::exists(metaFile))
        return;

    try
    {
        boost::property_tree::ptree metaTree;
        boost::property_tree::read_json(metaFile.string(), metaTree);

        const auto regionsNode = metaTree.get_child_optional("regions");
        if (!regionsNode)
            return;

        for (const auto& entry : *regionsNode)
        {
            const auto name = entry.second.get_optional<std::string>("name");
            const auto startTime = entry.second.get_optional<float>("startTime");
            const auto endTime = entry.second.get_optional<float>("endTime");
            if (!name || !startTime || !endTime)
            {
                std::cout << "[Animation] Skipping invalid region in " << metaFile << std::endl;
                continue;
            }

            regions.push_back(AnimationRegion{*name, *startTime, *endTime});
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "[Animation] Failed to load regions from meta: " << ex.what() << std::endl;
    }
}

bool Animation::saveRegionsToMeta()
{
    auto filesMap = getEngineRegistryFilesMap();
    const auto found = filesMap.find(getAssetId());
    if (found == filesMap.end())
        return false;

    const fs::path contentPath = found->second;
    const fs::path metaFile = contentPath.parent_path() / (getAssetId() + ".meta.json");

    boost::property_tree::ptree metaTree;
    try
    {
        if (fs::exists(metaFile))
            boost::property_tree::read_json(metaFile.string(), metaTree);
    }
    catch (const std::exception& ex)
    {
        std::cout << "[Animation] Failed to parse meta before save, creating fresh tree: " << ex.what() << std::endl;
        metaTree.clear();
    }

    if (!metaTree.get_optional<std::string>("guid"))
        metaTree.put("guid", getAssetId());
    if (!metaTree.get_optional<std::string>("type"))
        metaTree.put("type", "animation");
    if (!metaTree.get_optional<std::string>("version"))
        metaTree.put("version", "1");
    if (!metaTree.get_optional<std::string>("content.relative_path"))
        metaTree.put("content.relative_path", contentPath.filename().string());

    boost::property_tree::ptree regionsTree;
    for (const auto& region : regions)
    {
        boost::property_tree::ptree regionTree;
        regionTree.put("name", region.name);
        regionTree.put("startTime", region.startTime);
        regionTree.put("endTime", region.endTime);
        regionsTree.push_back(std::make_pair("", regionTree));
    }

    metaTree.put_child("regions", regionsTree);

    try
    {
        boost::property_tree::write_json(metaFile.string(), metaTree);
        return true;
    }
    catch (const std::exception& ex)
    {
        std::cout << "[Animation] Failed to save regions to meta: " << ex.what() << std::endl;
        return false;
    }
}

void Animation::saveContent(fs::path contentFile, std::ostream& os)
{
    fs::path dir = fs::path(contentFile.string()).parent_path();
    if (dir.empty()) {
        // Set the directory to the current working directory
        dir = fs::current_path();
    }
    if (!fs::exists(dir)) {
        if (!fs::create_directories(dir)) {
            std::cerr << "Failed to create directories: " << dir << std::endl;
            return;
        }
    }
    std::ofstream ofs(contentFile.string());
    if (!ofs.is_open()) {
        std::cerr << "Failed to open file for writing: " << contentFile << std::endl;
        return;
    }
    boost::archive::text_oarchive oa(ofs);
    oa << *this;
    ofs.close();
}

void Animation::loadContent(fs::path contentFile, std::istream& is)
{
    std::ifstream ifs(contentFile.string());
    boost::archive::text_iarchive ia(ifs);
    ia >> *this;
}