#pragma once
#include <sol/sol.hpp>
#include <stdexcept>
#include <string>
#include <functional>
#include <sstream>
#include <iostream>

class LuaEngine {
public:
    LuaEngine() { open(); }
    sol::state& state() { return lua_; }

    // Load any Lua chunk and return the callable chunk function.
    sol::protected_function loadChunk(const std::string& src,
                                      const std::string& debugName = "chunk") {
        sol::load_result lr = lua_.load(src, debugName);
        if (!lr.valid()) {
            sol::error err = lr;
            throw std::runtime_error(std::string("Lua load error: ") + err.what());
        }
        return lr;
    }

    // Execute a chunk and return the raw protected result (can contain returns).
    sol::protected_function_result runChunk(const std::string& src,
                                            const std::string& debugName = "chunk") {
        sol::protected_function chunk = loadChunk(src, debugName);
        sol::protected_function_result r = chunk();
        if (!r.valid()) {
            sol::error err = r;
            throw std::runtime_error(std::string("Lua runtime error: ") + err.what());
        }
        return r;
    }

    // Compile a chunk that *returns a function* and give you that function.
    sol::protected_function loadFunction(const std::string& src,
                                         const std::string& debugName = "chunk") {
        sol::protected_function_result r = runChunk(src, debugName);
        sol::object fn = r;
        if (fn.get_type() != sol::type::function) {
            throw std::runtime_error("Lua chunk did not return a function");
        }
        return fn.as<sol::protected_function>();
    }

    // Optional: redirect Lua print(...) to a host callback (e.g., engine console).
    void setPrintHandler(std::function<void(const std::string&)> handler) {
        printHandler_ = std::move(handler);
        if (!printHandler_) {
            lua_["print"] = defaultPrint_;
            return;
        }
        lua_.set_function("print", [this](sol::variadic_args args) {
            std::ostringstream oss;
            sol::protected_function tostring = lua_["tostring"];
            bool first = true;
            for (auto v : args) {
                sol::protected_function_result tr = tostring(v);
                std::string s = tr.valid() ? tr.get<std::string>() : "<tostring error>";
                if (!first) oss << "\t";
                oss << s;
                first = false;
            }
            printHandler_(oss.str());
        });
    }

private:
    sol::state lua_;
    sol::protected_function defaultPrint_;
    std::function<void(const std::string&)> printHandler_;

    void open() {
        lua_.open_libraries(sol::lib::base, sol::lib::math,
                            sol::lib::table, sol::lib::string);
        defaultPrint_ = lua_["print"];
        setPrintHandler([](const std::string& line) {
            std::cout << "[LUA] " << line << std::endl;
        });
    }
};
