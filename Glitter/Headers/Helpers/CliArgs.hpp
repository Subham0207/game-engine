#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Lightweight, header-only CLI argument parsing.
//
// Supported forms:
//   --flag
//   --key=value
//   --key value
//   -f               (short flag)
//   -k value         (short option)
//   -abc             (short flag bundle => -a -b -c)
//   --               end of options; remaining tokens become positional
//
// Notes:
//  - By default, unknown options are an error. You can relax this with setAllowUnknown(true).
//  - Options must be registered via addFlag/addOption if you want help text and strict validation.

namespace Glitter::Helpers {

class CliArgs {
public:
    enum class Arity {
        None,  // flag
        One    // option requires a value
    };

    struct Spec {
        std::string longName;              // without leading --
        char shortName = 0;                // without leading - (0 = none)
        Arity arity = Arity::None;
        std::string valueHint;             // e.g. "PATH"
        std::string description;
        std::optional<std::string> defaultValue; // only meaningful for Arity::One
    };

    struct Result {
        bool ok = true;
        std::string error;
        bool wantsHelp = false;
    };

    CliArgs() = default;

    // Registration (optional but recommended).
    // If you register specs and keep allowUnknown=false (default), parser will validate.
    void addFlag(std::string longName,
                 char shortName = 0,
                 std::string description = {})
    {
        Spec s;
        s.longName = std::move(longName);
        s.shortName = shortName;
        s.arity = Arity::None;
        s.description = std::move(description);
        registerSpec(std::move(s));
    }

    void addOption(std::string longName,
                   char shortName = 0,
                   std::string valueHint = "VALUE",
                   std::string description = {},
                   std::optional<std::string> defaultValue = std::nullopt)
    {
        Spec s;
        s.longName = std::move(longName);
        s.shortName = shortName;
        s.arity = Arity::One;
        s.valueHint = std::move(valueHint);
        s.description = std::move(description);
        s.defaultValue = std::move(defaultValue);
        registerSpec(std::move(s));
    }

    // Common pattern: automatically provide -h/--help.
    void addHelpFlag(std::string description = "Show this help message")
    {
        addFlag("help", 'h', std::move(description));
    }

    void setAllowUnknown(bool allow) { m_allowUnknown = allow; }

    // Parse argv. Call once, then query.
    Result parse(int argc, const char* const* argv)
    {
        resetParsedState();

        if (argc > 0 && argv && argv[0]) {
            m_program = argv[0];
        }

        // Apply defaults for registered options.
        for (const auto& s : m_specs) {
            if (s.arity == Arity::One && s.defaultValue.has_value()) {
                m_values[s.longName] = *s.defaultValue;
            }
        }

        Result res;

        bool endOfOptions = false;
        for (int i = 1; i < argc; ++i) {
            std::string_view tok = argv[i] ? std::string_view(argv[i]) : std::string_view();

            if (!endOfOptions) {
                if (tok == "--") {
                    endOfOptions = true;
                    continue;
                }

                if (startsWith(tok, "--")) {
                    auto r = parseLong(tok, i, argc, argv, res);
                    if (!r.ok) return r;
                    continue;
                }

                if (startsWith(tok, "-") && tok.size() > 1) {
                    auto r = parseShort(tok, i, argc, argv, res);
                    if (!r.ok) return r;
                    continue;
                }
            }

            m_positionals.emplace_back(tok);
        }

        // help?
        if (hasFlag("help")) {
            res.wantsHelp = true;
        }

        return res;
    }

    // Queries
    bool hasFlag(std::string_view longName) const
    {
        auto it = m_flags.find(std::string(longName));
        return it != m_flags.end();
    }

    std::optional<std::string> get(std::string_view longName) const
    {
        auto it = m_values.find(std::string(longName));
        if (it == m_values.end()) return std::nullopt;
        return it->second;
    }

    std::string getString(std::string_view longName, std::string defaultValue = {}) const
    {
        auto v = get(longName);
        return v ? *v : defaultValue;
    }

    bool getBool(std::string_view longName, bool defaultValue = false) const
    {
        if (hasFlag(longName)) return true;
        auto v = get(longName);
        if (!v) return defaultValue;
        return parseBool(*v).value_or(defaultValue);
    }

    int getInt(std::string_view longName, int defaultValue = 0) const
    {
        auto v = get(longName);
        if (!v) return defaultValue;
        char* end = nullptr;
        long val = std::strtol(v->c_str(), &end, 10);
        if (!end || *end != '\0') return defaultValue;
        return static_cast<int>(val);
    }

    float getFloat(std::string_view longName, float defaultValue = 0.0f) const
    {
        auto v = get(longName);
        if (!v) return defaultValue;
        char* end = nullptr;
        float val = std::strtof(v->c_str(), &end);
        if (!end || *end != '\0') return defaultValue;
        return val;
    }

    const std::vector<std::string>& positionals() const { return m_positionals; }

    // Text helpers
    std::string program() const { return m_program; }

    std::string usage(std::string_view header = {}, std::string_view footer = {}) const
    {
        std::ostringstream oss;

        if (!header.empty()) {
            oss << header << "\n\n";
        }

        oss << "Usage: ";
        if (!m_program.empty()) {
            oss << m_program;
        } else {
            oss << "<program>";
        }
        oss << " [options]";
        if (!m_positionals.empty()) {
            oss << " [--] [args...]";
        }
        oss << "\n\n";

        if (!m_specs.empty()) {
            oss << "Options:\n";

            // compute padding
            size_t pad = 0;
            for (const auto& s : m_specs) {
                pad = std::max(pad, optionDisplay(s).size());
            }
            pad = std::min<size_t>(pad, 42);

            for (const auto& s : m_specs) {
                auto left = optionDisplay(s);
                oss << "  " << left;
                if (left.size() < pad) {
                    oss << std::string(pad - left.size(), ' ');
                }
                oss << "  " << s.description;
                if (s.arity == Arity::One && s.defaultValue.has_value()) {
                    oss << " (default: " << *s.defaultValue << ")";
                }
                oss << "\n";
            }
        }

        if (!footer.empty()) {
            oss << "\n" << footer << "\n";
        }

        return oss.str();
    }

    // Access to registered specs (for external help formatting, etc.)
    const std::vector<Spec>& specs() const { return m_specs; }

private:
    void resetParsedState()
    {
        m_program.clear();
        m_flags.clear();
        m_values.clear();
        m_positionals.clear();
    }

    void registerSpec(Spec s)
    {
        normalizeName(s.longName);
        if (s.shortName != 0 && !isShortNameValid(s.shortName)) {
            throw std::invalid_argument("CliArgs: invalid short option name");
        }

        // Ensure uniqueness.
        for (const auto& existing : m_specs) {
            if (existing.longName == s.longName && !s.longName.empty()) {
                throw std::invalid_argument("CliArgs: duplicate long option name: " + s.longName);
            }
            if (s.shortName != 0 && existing.shortName == s.shortName) {
                throw std::invalid_argument("CliArgs: duplicate short option name");
            }
        }

        m_specs.emplace_back(std::move(s));
    }

    const Spec* findLong(std::string_view longName) const
    {
        std::string key(longName);
        normalizeName(key);
        for (const auto& s : m_specs) {
            if (s.longName == key) return &s;
        }
        return nullptr;
    }

    const Spec* findShort(char shortName) const
    {
        for (const auto& s : m_specs) {
            if (s.shortName == shortName) return &s;
        }
        return nullptr;
    }

    Result parseLong(std::string_view tok, int& i, int argc, const char* const* argv, Result res)
    {
        // tok begins with "--"
        std::string_view body = tok.substr(2);
        if (body.empty()) {
            res.ok = false;
            res.error = "Invalid option: --";
            return res;
        }

        std::string_view name = body;
        std::optional<std::string_view> value;

        auto eq = body.find('=');
        if (eq != std::string_view::npos) {
            name = body.substr(0, eq);
            value = body.substr(eq + 1);
        }

        std::string nameStr(name);
        normalizeName(nameStr);

        if (nameStr == "help") {
            m_flags.insert(nameStr);
            return res;
        }

        const Spec* spec = findLong(nameStr);
        if (!spec && !m_allowUnknown && !m_specs.empty()) {
            res.ok = false;
            res.error = "Unknown option: --" + nameStr;
            return res;
        }

        Arity arity = spec ? spec->arity : (value.has_value() ? Arity::One : Arity::None);

        if (arity == Arity::None) {
            if (value.has_value() && !value->empty()) {
                // treat --flag=value as setting value for unknown? That's ambiguous; disallow for flags.
                if (spec && spec->arity == Arity::None) {
                    res.ok = false;
                    res.error = "Option --" + nameStr + " does not take a value";
                    return res;
                }
            }
            m_flags.insert(nameStr);
            return res;
        }

        // Arity::One
        std::string val;
        if (value.has_value()) {
            val = std::string(*value);
        } else {
            if (i + 1 >= argc) {
                res.ok = false;
                res.error = "Option --" + nameStr + " requires a value";
                return res;
            }
            ++i;
            val = argv[i] ? std::string(argv[i]) : std::string();
        }
        m_values[nameStr] = std::move(val);
        return res;
    }

    Result parseShort(std::string_view tok, int& i, int argc, const char* const* argv, Result res)
    {
        // tok begins with "-" and size>1
        std::string_view body = tok.substr(1);
        if (body.empty()) {
            res.ok = false;
            res.error = "Invalid option: -";
            return res;
        }

        // Special: -h
        if (body.size() == 1 && body[0] == 'h') {
            m_flags.insert("help");
            return res;
        }

        // If registered spec expects value, allow -kVALUE? Keep minimal: only -k value.
        // Bundle flags: -abc => -a -b -c (only if all are flags)
        if (body.size() > 1) {
            // If first short is an option requiring value and not bundled, we still require separated value.
            bool allFlags = true;
            for (char c : body) {
                const Spec* s = findShort(c);
                if (s && s->arity == Arity::One) {
                    allFlags = false;
                    break;
                }
                if (!s && !m_allowUnknown && !m_specs.empty()) {
                    allFlags = false;
                    break;
                }
            }

            if (allFlags) {
                for (char c : body) {
                    const Spec* s = findShort(c);
                    if (!s && (m_allowUnknown || m_specs.empty())) {
                        // unknown short treated as flag with name = "-x"
                        m_flags.insert(std::string(1, c));
                    } else if (s) {
                        m_flags.insert(s->longName);
                    }
                }
                return res;
            }
        }

        // Single short option.
        char c = body[0];
        const Spec* spec = findShort(c);
        if (!spec && !m_allowUnknown && !m_specs.empty()) {
            res.ok = false;
            res.error = std::string("Unknown option: -") + c;
            return res;
        }

        if (!spec) {
            // unknown; treat as flag
            m_flags.insert(std::string(1, c));
            return res;
        }

        if (spec->arity == Arity::None) {
            m_flags.insert(spec->longName);
            return res;
        }

        // requires value
        if (i + 1 >= argc) {
            res.ok = false;
            res.error = std::string("Option -") + c + " requires a value";
            return res;
        }
        ++i;
        std::string val = argv[i] ? std::string(argv[i]) : std::string();
        m_values[spec->longName] = std::move(val);
        return res;
    }

    static bool startsWith(std::string_view s, std::string_view prefix)
    {
        return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
    }

    static void normalizeName(std::string& s)
    {
        // normalize to lower-case; keep '-' as-is
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
    }

    static bool isShortNameValid(char c)
    {
        return std::isalnum(static_cast<unsigned char>(c)) != 0;
    }

    static std::optional<bool> parseBool(const std::string& v)
    {
        std::string s = v;
        normalizeName(s);
        if (s == "1" || s == "true" || s == "yes" || s == "on") return true;
        if (s == "0" || s == "false" || s == "no" || s == "off") return false;
        return std::nullopt;
    }

    static std::string optionDisplay(const Spec& s)
    {
        std::ostringstream oss;
        if (s.shortName != 0) {
            oss << "-" << s.shortName;
            if (!s.longName.empty()) oss << ", ";
        }
        if (!s.longName.empty()) {
            oss << "--" << s.longName;
        }
        if (s.arity == Arity::One) {
            oss << " <" << (s.valueHint.empty() ? "VALUE" : s.valueHint) << ">";
        }
        return oss.str();
    }

private:
    bool m_allowUnknown = false;

    std::string m_program;
    std::vector<Spec> m_specs;

    std::set<std::string> m_flags;
    std::map<std::string, std::string> m_values;
    std::vector<std::string> m_positionals;
};

} // namespace Glitter::Helpers

// EXAMPLE USAGE:
// #include "Helpers/CliArgs.hpp"
//
// int main(int argc, char** argv)
// {
//     Glitter::Helpers::CliArgs cli;
//     cli.addHelpFlag();
//     cli.addFlag("headless", 'H', "Run without a window");
//     cli.addOption("project", 'p', "PATH", "Project file/folder", std::nullopt);
//     cli.addOption("width", 0, "PX", "Window width", "1280");
//     cli.addOption("height", 0, "PX", "Window height", "720");
//
//     auto res = cli.parse(argc, argv);
//     if (!res.ok) {
//         // print error + usage
//         std::cerr << res.error << "\n\n" << cli.usage() << std::endl;
//         return 1;
//     }
//     if (res.wantsHelp) {
//         std::cout << cli.usage() << std::endl;
//         return 0;
//     }
//
//     const bool headless = cli.getBool("headless");
//     const std::string project = cli.getString("project", "");
//     const int w = cli.getInt("width", 1280);
//     const int h = cli.getInt("height", 720);
//
//     // ...
// }
