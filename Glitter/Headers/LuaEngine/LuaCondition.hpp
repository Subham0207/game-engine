// LuaCondition.hpp
#pragma once
#include "LuaEngine.hpp"
#include <optional>
#include <string>
#include <serializeAClass.hpp>

class LuaCondition {
public:
    LuaCondition() = default;
    explicit LuaCondition(std::string source) : source_(std::move(source)) {}

    // Compile lazily
    void compile(LuaEngine& engine) {
        if (fn_.has_value()) return;
        fn_ = engine.loadFunction(source_, "LuaCondition");
    }

    template <typename... Args>
    bool evaluate(LuaEngine& engine, Args&&... args) {
        compile(engine);
        sol::protected_function& f = *fn_;
        sol::protected_function_result r = f(std::forward<Args>(args)...);
        if (!r.valid()) {
            sol::error err = r;
            throw std::runtime_error(std::string("Lua call error: ") + err.what());
        }
        return r.get<bool>();
    }

    const std::string& source() const { return source_; }
    void setSource(std::string source) { source_ = source;}

    // Optional: release ref early (usually not needed; destructor handles it)
    void reset() { fn_.reset(); }

private:
    std::string source_;
    std::optional<sol::protected_function> fn_;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & source_;
    }
};
