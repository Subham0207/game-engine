#pragma once
#include <sol/sol.hpp>
#include <stdexcept>
#include <string>

class LuaEngine {
public:
    LuaEngine() { open(); }
    sol::state& state() { return lua_; }

    // Compile a chunk that *returns a function* and give you that function.
    sol::protected_function loadFunction(const std::string& src,
                                         const std::string& debugName = "chunk") {
        sol::load_result lr = lua_.load(src, debugName);
        if (!lr.valid()) {
            sol::error err = lr;
            throw std::runtime_error(std::string("Lua load error: ") + err.what());
        }

        sol::protected_function factory = lr;
        sol::protected_function_result r = factory();
        if (!r.valid()) {
            sol::error err = r;
            throw std::runtime_error(std::string("Lua factory error: ") + err.what());
        }

        sol::object fn = r;
        if (fn.get_type() != sol::type::function) {
            throw std::runtime_error("Lua chunk did not return a function");
        }

        // Keep as protected_function (holds a registry ref under the hood)
        return fn.as<sol::protected_function>();
    }

private:
    sol::state lua_;
    void open() {
        lua_.open_libraries(sol::lib::base, sol::lib::math,
                            sol::lib::table, sol::lib::string);
    }
};