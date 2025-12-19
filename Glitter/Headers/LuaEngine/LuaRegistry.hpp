//
// Created by subha on 17-12-2025.
//

#ifndef GLITTER_LUAREGISTRY_HPP
#define GLITTER_LUAREGISTRY_HPP
#include <filesystem>
#include <typeindex>

#include "ILuaClassDef.hpp"
#include "LuaClassBuilder.hpp"
#include "luaStubs.hpp"
#include "RegisterBinding.hpp"
#include "sol/sol.hpp"
namespace fs = std::filesystem;

class LuaRegistry {
public:
    LuaRegistry() = default;
    ~LuaRegistry() = default;

    LuaRegistry(const LuaRegistry&) = delete;
    LuaRegistry& operator=(const LuaRegistry&) = delete;

    LuaRegistry(LuaRegistry&&) noexcept = default;
    LuaRegistry& operator=(LuaRegistry&&) noexcept = default;

    template <class T>
    LuaClassBuilder<T> beginClass(const std::string& luaName);

    template <typename T, typename... Ctors>
    LuaClassBuilder<T> beginClass(const std::string& luaName, sol::constructors<Ctors...>);

    LuaRegistry& addGlobal(const std::string& name, const std::string& type, const std::string& doc = {});

    // stub emission
    void writeStubs(const std::filesystem::path& outDir) const;

    // binding application
    void apply(sol::state& lua) const;

    static void writeLuaRC(const fs::path& projectRoot) ;

    static void SetupLua(sol::state& lua, const std::filesystem::path& projectRoot);
private:
    std::unordered_map<std::type_index, std::unique_ptr<ILuaClassDef>> m_classes;
    std::vector<std::type_index> m_classOrder;
    std::vector<LuaGlobalStub> m_globals;
};
#endif //GLITTER_LUAREGISTRY_HPP