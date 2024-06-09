#ifndef SERIALIZATION_GLM_HPP
#define SERIALIZATION_GLM_HPP

#include <boost/serialization/serialization.hpp>
#include <glm/glm.hpp>

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
void serialize(Archive &ar, glm::mat4 &mat, const unsigned int version) {
    ar & boost::serialization::make_array(&mat[0][0], 16);
}

}
}

#endif // SERIALIZATION_GLM_HPP