//
// Created by subha on 18-12-2025.
//

#ifndef GLITTER_LUACLASSBUILDER_HPP
#define GLITTER_LUACLASSBUILDER_HPP
#include "LuaClassDef .hpp"

template <typename T>
class LuaClassBuilder
{
public:
    explicit LuaClassBuilder(LuaClassDef<T>* def) : m_def(def) {}

    template <typename MemFn>
    LuaClassBuilder& method(const std::string& luaName, MemFn fn,
                            const std::string& emmyFunType,
                            const std::string& doc = {})
    {
        m_def->addBinder([luaName, fn](sol::usertype<T>& ut) { ut[luaName] = fn; });
        m_def->addMethodStub(luaName, emmyFunType, doc);
        return *this;
    }

    template <typename FieldPtr>
    LuaClassBuilder& field(const std::string& luaName, FieldPtr ptr,
                           const std::string& emmyType,
                           const std::string& doc = {})
    {
        m_def->addBinder([luaName, ptr](sol::usertype<T>& ut) { ut[luaName] = ptr; });
        m_def->addFieldStub(luaName, emmyType, doc);
        return *this;
    }

    template <typename Getter, typename Setter>
    LuaClassBuilder& property(const std::string& luaName, Getter g, Setter s,
                              const std::string& emmyType,
                              const std::string& doc = {})
    {
        m_def->addBinder([luaName, g, s](sol::usertype<T>& ut) {
            ut.set(luaName, sol::property(g, s));
        });
        m_def->addFieldStub(luaName, emmyType, doc);
        return *this;
    }

    template <typename Getter>
    LuaClassBuilder& property_readonly(const std::string& luaName, Getter g,
                                       const std::string& emmyType,
                                       const std::string& doc = {})
    {
        m_def->addBinder([luaName, g](sol::usertype<T>& ut) {
            ut.set(luaName, sol::property(g));
        });
        m_def->addFieldStub(luaName, emmyType, doc);
        return *this;
    }

private:
    LuaClassDef<T>* m_def;
};
#endif //GLITTER_LUACLASSBUILDER_HPP