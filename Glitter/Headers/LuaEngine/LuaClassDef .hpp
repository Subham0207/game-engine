//
// Created by subha on 18-12-2025.
//

#ifndef GLITTER_LUACLASSDEF_HPP
#define GLITTER_LUACLASSDEF_HPP
#include "ILuaClassDef.hpp"

template <typename T>
class LuaClassDef final : public ILuaClassDef
{
public:
    using BinderFn = std::function<void(sol::usertype<T>&)>;

    LuaClassDef(std::string luaName,
                std::function<sol::usertype<T>(sol::state&, const std::string&)> factory)
        : m_stub{std::move(luaName), {}, {}}
    , m_factory(std::move(factory))
    {}

    std::type_index type() const override { return std::type_index(typeid(T)); }
    const LuaClassStub& stub() const override { return m_stub; }

    void addBinder(BinderFn fn) { m_binders.emplace_back(std::move(fn)); }

    void addFieldStub(std::string name, std::string type, std::string doc = {})
    {
        m_stub.fields.push_back({std::move(name), std::move(type), std::move(doc)});
    }

    void addMethodStub(std::string name, std::string funType, std::string doc = {})
    {
        m_stub.methods.push_back({std::move(name), std::move(funType), std::move(doc)});
    }

    void apply(sol::state& lua) const override
    {
        sol::usertype<T> ut = m_factory(lua, m_stub.name);
        for (const auto& b : m_binders)
            b(ut);
    }

private:
    LuaClassStub m_stub;
    std::vector<BinderFn> m_binders;
    std::function<sol::usertype<T>(sol::state&, const std::string&)> m_factory;
};

#endif //GLITTER_LUACLASSDEF_HPP