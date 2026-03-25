//
// Created by subha on 25-03-2026.
//

#ifndef GLITTER_ATTRIBUTE_HPP
#define GLITTER_ATTRIBUTE_HPP
#pragma once
#include <string>

namespace NodeGraphComponents::Node
{
    enum TYPE
    {
        FIELD,
        PIN
    };

    class Attribute
    {
    public:
        Attribute(std::string name, const TYPE type) : name(std::move(name)), type(type) {}
        [[nodiscard]] std::string getName() const { return name; }
        [[nodiscard]] TYPE getType() const { return type; }
    private:
        std::string name;
        TYPE type;
    };
};
#endif //GLITTER_ATTRIBUTE_HPP