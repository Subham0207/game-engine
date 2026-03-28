//
// Created by subha on 28-03-2026.
//

#ifndef GLITTER_NODEGRAPHHELPERS_HPP
#define GLITTER_NODEGRAPHHELPERS_HPP
#pragma once
#include <boost/pfr.hpp>
#include <vector>
#include <sstream>
#include <boost/describe.hpp>
#include <boost/mp11.hpp>

namespace NodeGraphHelpers
{
    //Helper for below function
    template <typename T, std::size_t... I>
    std::vector<std::string> get_field_type_names_impl(std::index_sequence<I...>) {
        return { typeid(boost::pfr::tuple_element_t<I, T>).name()... };
    }

    //To get Field names in a Generic Type struct
    template <typename T>
    std::vector<std::string> get_field_type_names() {
        return get_field_type_names_impl<T>(
            std::make_index_sequence<boost::pfr::tuple_size_v<T>>{}
        );
    }

    //To get Field values from a Generic Typed struct object
    template <typename T>
    std::vector<std::string> get_field_value_strings(const T& value) {
        std::vector<std::string> results;
        boost::pfr::for_each_field(value, [&results](const auto& field) {
            std::ostringstream stream;
            stream << field;
            results.push_back(stream.str());
        });
        return results;
    }

    //To get variable names from Generic Type struct
    //Note to register fieldnames BOOST_DESCRIBE_STRUCT(Player, (), (health, name, isAlive, score)).
    //Player is struct and health, name, isAlive, score are in same order as in struct.
    template <typename T>
    std::vector<std::string> get_field_names() {
        std::vector<std::string> names;
        boost::mp11::mp_for_each<boost::describe::describe_members<T, boost::describe::mod_any_access>>(
            [&names](auto descriptor) {
                names.push_back(descriptor.name);
            }
        );
        return names;
    }
};


#endif //GLITTER_NODEGRAPHHELPERS_HPP