#include <3DModel/Animation/Animation.hpp>
#include <iostream>
#include <fstream> 

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