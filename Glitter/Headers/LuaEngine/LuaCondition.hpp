// LuaCondition.hpp
#pragma once
#include "LuaEngine.hpp"
#include <exception>
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

        const std::string normalized = normalizeSourceForFunction(source_);
        try
        {
            fn_ = engine.loadFunction(normalized, "LuaCondition");
            return;
        }
        catch (const std::exception&)
        {
            // Backward compatibility for raw chunks that don't already return a function.
            const std::string wrapped =
                std::string("return function(t)\n") + normalized + "\nreturn false\nend";
            fn_ = engine.loadFunction(wrapped, "LuaConditionWrapped");
        }
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

        const sol::object value = r.get<sol::object>();
        if (value.is<bool>())
            return value.as<bool>();
        if (value.is<int>())
            return value.as<int>() != 0;
        if (value.is<double>())
            return value.as<double>() != 0.0;
        return false;
    }

    const std::string& source() const { return source_; }
    void setSource(std::string source) { source_ = std::move(source); fn_.reset(); }

    // Optional: release ref early (usually not needed; destructor handles it)
    void reset() { fn_.reset(); }

private:
    std::string source_;
    std::optional<sol::protected_function> fn_;

    static std::string trimCopy(const std::string& input)
    {
        const auto begin = input.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos)
            return "";
        const auto end = input.find_last_not_of(" \t\r\n");
        return input.substr(begin, end - begin + 1);
    }

    static std::string normalizeSourceForFunction(const std::string& source)
    {
        const std::string trimmed = trimCopy(source);
        if (trimmed.empty())
            return "return function(t) return false end";
        return trimmed;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & source_;
    }
};
