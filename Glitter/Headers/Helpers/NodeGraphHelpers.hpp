//
// Created by subha on 28-03-2026.
//

#ifndef GLITTER_NODEGRAPHHELPERS_HPP
#define GLITTER_NODEGRAPHHELPERS_HPP
#pragma once
#include <boost/pfr.hpp>
#include <vector>
#include <sstream>
#include <string>
#include <type_traits>
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

    inline std::string escape_lua_string(const std::string& input)
    {
        std::string out;
        out.reserve(input.size() + 8);
        for (const char c : input)
        {
            switch (c)
            {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
            }
        }
        return out;
    }

    template <typename TValue>
    std::string to_lua_literal(const TValue& value)
    {
        using ValueType = std::decay_t<TValue>;
        if constexpr (std::is_same_v<ValueType, bool>)
        {
            return value ? "true" : "false";
        }
        else if constexpr (std::is_integral_v<ValueType> || std::is_floating_point_v<ValueType>)
        {
            std::ostringstream stream;
            stream << value;
            return stream.str();
        }
        else if constexpr (std::is_same_v<ValueType, std::string>)
        {
            return "\"" + escape_lua_string(value) + "\"";
        }
        else if constexpr (std::is_same_v<ValueType, const char*> || std::is_same_v<ValueType, char*>)
        {
            return value ? ("\"" + escape_lua_string(value) + "\"") : "\"\"";
        }
        else
        {
            return "nil";
        }
    }

    template <typename T>
    std::string get_field_name_or_default(const std::vector<std::string>& fieldNames, const std::size_t index)
    {
        if (index < fieldNames.size())
            return fieldNames[index];
        return "field_" + std::to_string(index);
    }

    template <typename T, std::size_t... I>
    void append_lua_fields(const T& value,
                           const std::vector<std::string>& fieldNames,
                           const std::string& objectName,
                           std::ostringstream& stream,
                           std::index_sequence<I...>)
    {
        ((stream << objectName << "." << get_field_name_or_default<T>(fieldNames, I)
                 << " = " << to_lua_literal(boost::pfr::get<I>(value)) << "\n"), ...);
    }

    // Build a Lua snippet that creates `objectName` and assigns reflected fields.
    template <typename T>
    std::string build_lua_object_prelude(const T& value, const std::string& objectName = "t")
    {
        std::ostringstream stream;
        stream << "local " << objectName << " = {}\n";

        const auto fieldNames = get_field_names<T>();
        append_lua_fields(
            value,
            fieldNames,
            objectName,
            stream,
            std::make_index_sequence<boost::pfr::tuple_size_v<T>>{}
        );

        return stream.str();
    }
};


#endif //GLITTER_NODEGRAPHHELPERS_HPP

