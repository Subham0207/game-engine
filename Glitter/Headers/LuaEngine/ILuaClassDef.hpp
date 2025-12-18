//
// Created by subha on 18-12-2025.
//

#ifndef GLITTER_ILUACLASSDEF_HPP
#define GLITTER_ILUACLASSDEF_HPP
#include <typeindex>

#include "luaStubs.hpp"
#include "sol/sol.hpp"

class ILuaClassDef
{
public:
    virtual ~ILuaClassDef() = default;
    virtual std::type_index type() const = 0;
    virtual const LuaClassStub& stub() const = 0;
    virtual void apply(sol::state& lua) const = 0;
};

#endif //GLITTER_ILUACLASSDEF_HPP