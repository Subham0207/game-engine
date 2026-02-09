#include <serializable.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>         
#include <fstream>
#include <EngineState.hpp>
#include <filesystem>
namespace fs = std::filesystem;
namespace bs = boost::property_tree;

Serializable::Serializable()
{
    generate_instance_guid();
}

void Serializable::save(fs::path &assetRoot)
{
    if(asset_guid_.empty())
        generate_asset_guid();
    // 1) ensure folder exists
    fs::create_directories(assetRoot);

    // 2) choose a content filename (child decides the extension)
    std::string contentNameWithExtension = contentName() + "." + typeName();
    const fs::path contentFile = assetRoot / contentNameWithExtension;

    // 3) give child an ostream to serialize *its* content only
    {
        std::ofstream os(contentFile, std::ios::binary);
        bs::ptree contentInJson;
        if (!os) throw std::runtime_error("Failed to open content file: " + contentFile.string());
        saveContent(contentFile, os);
        os.flush();
        os.close();
    }

    // 4) write meta (parent-owned)
    bs::ptree meta;
    meta.put("guid", asset_guid_);
    meta.put("type", typeName());
    meta.put("version", version_);
    meta.put("content.relative_path", contentFile.filename().string());

    const fs::path metaFile = assetRoot / (asset_guid_ +  ".meta.json");
    write_json(metaFile.string(), meta);

    EngineState::state->engineRegistry->update(asset_guid_, contentFile.string());
}

void Serializable::generate_asset_guid()
{
    asset_guid_ = boost::uuids::to_string(boost::uuids::random_generator()());
}

void Serializable::generate_instance_guid()
{
    instance_guid_ = boost::uuids::to_string(boost::uuids::random_generator()());
}

std::string Serializable::getAssetId()
{
    return asset_guid_;
}

std::string Serializable::getInstanceId()
{
    return instance_guid_;
}

void Serializable::setInstanceId(std::string id){instance_guid_ = id;}
void Serializable::setAssetId(std::string id){asset_guid_ = id;}

void Serializable::load(fs::path& assetRoot, std::string filename) {

    // 1) read meta
    const fs::path metaFile = fs::path(EngineState::state->currentActiveProjectDirectory) / assetRoot / (filename + ".meta.json");
    bs::ptree meta;
    read_json(metaFile.string(), meta);
    
    // (Optionally) validate type/version here
    asset_guid_ = meta.get<std::string>("guid");
    
    // 2) load content (child only reads its payload)
    const fs::path contentRel = meta.get<std::string>("content.relative_path");
    const fs::path contentFile = fs::path(EngineState::state->currentActiveProjectDirectory) / assetRoot / contentRel;
    
    std::cout << "Loading: " << contentFile.string() << std::endl;

    std::ifstream is(contentFile, std::ios::binary);
    if (!is) throw std::runtime_error("Failed to open content file: " + contentFile.string());
    loadContent(contentFile,is);
}

void Serializable::deleteFile()
{
    auto filesMap = getEngineRegistryFilesMap();
    auto assetRoot = fs::path(filesMap[asset_guid_]).parent_path();

    const fs::path metaFile = fs::path(EngineState::state->currentActiveProjectDirectory) / assetRoot / (asset_guid_ + ".meta.json");
    bs::ptree meta;
    read_json(metaFile.string(), meta);

    const fs::path contentRel = meta.get<std::string>("content.relative_path");
    const fs::path contentFile = fs::path(EngineState::state->currentActiveProjectDirectory) / assetRoot / contentRel;
    try {
        std::filesystem::remove(metaFile);
        std::filesystem::remove(contentFile);
    }
    catch(const std::filesystem::filesystem_error& err) {
        std::cout << "filesystem error: " << err.what() << '\n';
    }

}
