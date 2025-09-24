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

void Serializable::save(fs::path &assetRoot)
{
    guid_ = boost::uuids::to_string(boost::uuids::random_generator()()); 
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
    }

    // 4) write meta (parent-owned)
    bs::ptree meta;
    meta.put("guid", guid_);
    meta.put("type", typeName());
    meta.put("version", version_);
    meta.put("content.relative_path", contentFile.filename().string());

    const fs::path metaFile = assetRoot / (contentName() + ".meta.json");
    write_json(metaFile.string(), meta);
}

std::string Serializable::guid()
{
    return boost::uuids::to_string(boost::uuids::random_generator()());
}

std::string Serializable::getGUID()
{
    return guid_;
}

void Serializable::load(fs::path& assetRoot, std::string filename) {
    // 1) read meta
    const fs::path metaFile = fs::path(State::state->currentActiveProjectDirectory) / assetRoot / (filename + ".meta.json");
    bs::ptree meta;
    read_json(metaFile.string(), meta);

    // (Optionally) validate type/version here
    guid_ = meta.get<std::string>("guid");

    // 2) load content (child only reads its payload)
    const fs::path contentRel = meta.get<std::string>("content.relative_path");
    const fs::path contentFile = fs::path(State::state->currentActiveProjectDirectory) / assetRoot / contentRel;

    std::ifstream is(contentFile, std::ios::binary);
    if (!is) throw std::runtime_error("Failed to open content file: " + contentFile.string());
    loadContent(contentFile,is);
}