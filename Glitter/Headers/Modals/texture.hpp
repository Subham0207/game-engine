#pragma once
#include "assimp/scene.h"
#include "serializer.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/access.hpp"

namespace ProjectModals{
    struct Texture {
    unsigned int id;
    aiTextureType type;
    std::string name;

    Texture()=default;
    Texture(unsigned int textureId, aiTextureType textureType, std::string filename)
        : id(textureId), type(textureType), name(filename) {}

    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & id;
            ar & type;
            ar & name;
        }
    };
}