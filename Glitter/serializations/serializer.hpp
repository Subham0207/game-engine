#ifndef SERIALIZATION_GLM_HPP
#define SERIALIZATION_GLM_HPP

#pragma once
#include <boost/serialization/serialization.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/split_member.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <assimp/scene.h>

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive &ar, glm::vec2 &vec, const unsigned int version) {
    ar & vec.x;
    ar & vec.y;
}


template<class Archive>
void serialize(Archive &ar, glm::vec3 &vec, const unsigned int version) {
    ar & vec.x;
    ar & vec.y;
    ar & vec.z;
}

template<class Archive>
void serialize(Archive &ar, glm::vec4 &vec, const unsigned int version) {
    ar & vec.x;
    ar & vec.y;
    ar & vec.z;
    ar & vec.w;
}

template<class Archive>
void serialize(Archive &ar, glm::quat &rot, const unsigned int version) {
    ar & rot.w;
    ar & rot.x;
    ar & rot.y;
    ar & rot.z;
}

template<class Archive>
void serialize(Archive &ar, glm::mat4 &mat, const unsigned int version) {
    ar & boost::serialization::make_array(&mat[0][0], 16);
}

template<class Archive>
void serialize(Archive &ar, aiTextureType &type, const unsigned int version) {
    ar & boost::serialization::make_nvp("type", type);
}

template<class Archive>
void save(Archive & ar, const std::vector<glm::mat4*> &matrices, const unsigned int version) {
    size_t size = matrices.size();
    ar & size;

    for (size_t i = 0; i < size; ++i) {
        ar & boost::serialization::make_array(&(*matrices[i])[0][0], 16);
    }
}

template<class Archive>
void load(Archive & ar, std::vector<glm::mat4*> &matrices, const unsigned int version) {
    size_t size;
    ar & size;

    matrices.resize(size);
    for (size_t i = 0; i < size; ++i) {
        matrices[i] = new glm::mat4();
        ar & boost::serialization::make_array(&(*matrices[i])[0][0], 16);
    }
}

template<class Archive>
void serialize(Archive & ar, std::vector<glm::mat4*> &matrices, const unsigned int version) {
     boost::serialization::split_free(ar, matrices, version);
}


}
}

#endif // SERIALIZATION_GLM_HPP