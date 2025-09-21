#pragma once
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

class Serializable
{
    public:
        virtual void save(fs::path &assetRoot);
        virtual void load(fs::path& assetRoot, std::string filename);
    protected:
        virtual void saveContent(std::ostream& os) const = 0;
        virtual void loadContent(std::istream& is)       = 0;

        virtual const std::string typeName()          const = 0;
        virtual const std::string contentName()  const = 0;

        std::string guid();
    private:
        std::string version_ = "1";
        std::string guid_;
};