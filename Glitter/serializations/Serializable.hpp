#pragma once
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

class Serializable
{
    public:
    virtual ~Serializable() = default;
    Serializable();
        virtual void save(fs::path &assetRoot);
        virtual void load(fs::path& assetRoot, std::string filename);
        virtual void deleteFile();
        std::string getAssetId();
        
        std::string getInstanceId();
        void setInstanceId(std::string id);
        void setAssetId(std::string id);
        virtual const std::string contentName() = 0;

        virtual std::string GetClassId() const = 0;
    protected:
        virtual void saveContent(fs::path contentFileLocation, std::ostream& os) = 0;
        virtual void loadContent(fs::path contentFileLocation, std::istream& is) = 0;

        virtual const std::string typeName()          const = 0;

        void generate_asset_guid();// unqiue idenfier for serializable on disk.
        void generate_instance_guid();// unique identifier for serializable within a level.
    private:
        std::string version_ = "1";
        std::string asset_guid_;
        std::string instance_guid_;
};