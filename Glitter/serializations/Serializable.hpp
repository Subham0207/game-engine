#pragma once
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

class Serializable
{
    public:
        virtual void save(fs::path &assetRoot);
        virtual void load(fs::path& assetRoot, std::string filename);
        std::string getGUID();
    protected:
        virtual void saveContent(fs::path contentFileLocation, std::ostream& os) = 0;
        virtual void loadContent(fs::path contentFileLocation, std::istream& is) = 0;

        virtual const std::string typeName()          const = 0;
        virtual const std::string contentName() = 0;

        std::string guid();
    private:
        std::string version_ = "1";
        std::string guid_;
};