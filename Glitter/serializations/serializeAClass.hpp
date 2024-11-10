#pragma once
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/access.hpp>

#include <serializer.hpp>

// #define DECLARE_SERIALIZATION(...)                                  \
//     friend class boost::serialization::access;                      \
//     template<class Archive>                                         \
//     void serialize(Archive &ar, const unsigned int version) {       \
//         expandArgs(ar, __VA_ARGS__);                                \
//     }                                                               \
//     template <typename Archive, typename T>                         \
//     void expandArgs(Archive& ar, T&& t) { ar & t; }                 \
//     template <typename Archive, typename T, typename... Args>       \
//     void expandArgs(Archive& ar, T&& t, Args&&... args) {           \
//         ar & t;                                                     \
//         expandArgs(ar, std::forward<Args>(args)...);                \
//     }