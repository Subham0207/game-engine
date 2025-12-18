//
// Created by subha on 17-12-2025.
//

#ifndef GLITTER_LUAREGISTRY_HPP
#define GLITTER_LUAREGISTRY_HPP
#include <filesystem>
#include <typeindex>

#include "ILuaClassDef.hpp"
#include "luaStubs.hpp"
#include "sol/sol.hpp"

class LuaRegistry {
public:
    LuaRegistry() = default;
    ~LuaRegistry() = default;

    LuaRegistry(const LuaRegistry&) = delete;
    LuaRegistry& operator=(const LuaRegistry&) = delete;

    LuaRegistry(LuaRegistry&&) noexcept = default;
    LuaRegistry& operator=(LuaRegistry&&) noexcept = default;

    template <class T>
    LuaClassStub& beginClass(const std::string& luaName);

    LuaRegistry& addGlobal(const std::string& name, const std::string& type, const std::string& doc = {});

    // stub emission
    void writeStubs(const std::filesystem::path& outDir) const;

    // binding application
    void apply(sol::state& lua) const;
private:
    std::unordered_map<std::type_index, std::unique_ptr<ILuaClassDef>> m_classes;
    std::vector<std::type_index> m_classOrder;
    std::vector<LuaGlobalStub> m_globals;
};
#endif //GLITTER_LUAREGISTRY_HPP