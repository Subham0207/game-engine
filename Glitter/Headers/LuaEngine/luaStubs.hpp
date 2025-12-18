//
// Created by subha on 18-12-2025.
//

#ifndef GLITTER_LUASTUBS_HPP
#define GLITTER_LUASTUBS_HPP
#include <string>
#include <vector>

struct LuaMethodStub
{
    std::string name;     // e.g. "setMovement"
    std::string funType;  // e.g. "fun(self: PlayerController, dir: Vec3)"
    std::string doc;      // optional
};

struct LuaFieldStub
{
    std::string name; // e.g. "speed"
    std::string type; // e.g. "number"
    std::string doc;  // optional
};

struct LuaClassStub
{
    std::string name; // class name in Lua
    std::vector<LuaFieldStub> fields;
    std::vector<LuaMethodStub> methods;
};

struct LuaGlobalStub
{
    std::string name; // e.g. "Engine"
    std::string type; // e.g. "Engine"
    std::string doc;  // optional
};

#endif //GLITTER_LUASTUBS_HPP