//
// Created by subha on 25-03-2026.
//

#ifndef GLITTER_ATTRIBUTE_HPP
#define GLITTER_ATTRIBUTE_HPP
#pragma once
#include <string>

#define SIZE 50

namespace NodeGraphComponents::Node
{
    enum TYPE
    {
        FIELD,
        PIN,
        ExecutionFlowInPin,
        ExecutionFlowOutPin,
    };

    class Attribute
    {
    public:
        Attribute() = default;
        Attribute(int id, std::string name, const TYPE type) : m_id(id), name(std::move(name)), type(type), valueBuff("") {}
        [[nodiscard]] int getId() const { return m_id; }
        [[nodiscard]] std::string getName() const { return name; }
        [[nodiscard]] TYPE getType() const { return type; }

        //Only For Field type attributes;
        char* getValueBuff() { return valueBuff; }
        static int getValueSize() { return SIZE; }
    private:
        int m_id;
        std::string name;
        TYPE type;

        char valueBuff[SIZE];
    };
};
#endif //GLITTER_ATTRIBUTE_HPP