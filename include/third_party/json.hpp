// ============================================================================
// OFFLINE STAND-IN for nlohmann/json (NOT the real library).
// The spec (v2 FIX 6) requires nlohmann/json v3.11.3 single header. This sandbox
// had no network, so this file implements ONLY the API subset the game uses:
//   json::parse(istream|string), at(key), at(index), contains(key), size(),
//   is_array/is_object/is_string/is_number/is_boolean, get<T>() for
//   std::string/bool/int/float/double, and range-for over arrays.
// TO SHIP: overwrite this file with the real json.hpp from
//   https://github.com/nlohmann/json/releases/tag/v3.11.3 . No code changes needed.
// ============================================================================
#pragma once
#include <cstdlib>
#include <istream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace nlohmann {

class basic_json {
public:
    struct exception : std::runtime_error { using std::runtime_error::runtime_error; };
    enum class Type { Null, Bool, Number, String, Array, Object };

    basic_json() = default;

    static basic_json parse(const std::string& text) {
        size_t pos = 0;
        basic_json v = ParseValue(text, pos);
        SkipWs(text, pos);
        if (pos != text.size()) Fail("unexpected trailing characters", pos);
        return v;
    }
    static basic_json parse(std::istream& in) {
        std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        return parse(text);
    }

    bool is_object() const { return m_type == Type::Object; }
    bool is_array() const { return m_type == Type::Array; }
    bool is_string() const { return m_type == Type::String; }
    bool is_number() const { return m_type == Type::Number; }
    bool is_boolean() const { return m_type == Type::Bool; }

    size_t size() const {
        if (m_type == Type::Array) return m_items.size();
        if (m_type == Type::Object) return m_items.size();
        return 0;
    }
    bool contains(const std::string& key) const { return Find(key) != nullptr; }

    const basic_json& at(const std::string& key) const {
        const basic_json* p = Find(key);
        if (!p) throw exception("key '" + key + "' not found");
        return *p;
    }
    const basic_json& at(const char* key) const { return at(std::string(key)); }
    const basic_json& at(size_t index) const {
        if (m_type != Type::Array || index >= m_items.size()) throw exception("array index out of range");
        return m_items[index];
    }

    template <typename T> T get() const {
        if constexpr (std::is_same_v<T, std::string>) {
            if (m_type != Type::String) throw exception("value is not a string");
            return m_string;
        } else if constexpr (std::is_same_v<T, bool>) {
            if (m_type != Type::Bool) throw exception("value is not a boolean");
            return m_bool;
        } else {
            static_assert(std::is_arithmetic_v<T>, "unsupported get<T>()");
            if (m_type != Type::Number) throw exception("value is not a number");
            return static_cast<T>(m_number);
        }
    }

    std::vector<basic_json>::const_iterator begin() const { return m_items.begin(); }
    std::vector<basic_json>::const_iterator end() const { return m_items.end(); }

private:
    Type m_type = Type::Null;
    bool m_bool = false;
    double m_number = 0.0;
    std::string m_string;
    std::vector<basic_json> m_items;       // array elements or object values
    std::vector<std::string> m_keys;       // object keys (parallel to m_items)

    const basic_json* Find(const std::string& key) const {
        if (m_type != Type::Object) return nullptr;
        for (size_t i = 0; i < m_keys.size(); ++i)
            if (m_keys[i] == key) return &m_items[i];
        return nullptr;
    }
    [[noreturn]] static void Fail(const std::string& msg, size_t pos) {
        throw exception("parse error at byte " + std::to_string(pos) + ": " + msg);
    }
    static void SkipWs(const std::string& s, size_t& p) {
        while (p < s.size() && (s[p] == ' ' || s[p] == '\t' || s[p] == '\n' || s[p] == '\r')) ++p;
    }
    static std::string ParseString(const std::string& s, size_t& p) {
        if (s[p] != '"') Fail("expected string", p);
        ++p;
        std::string out;
        while (p < s.size() && s[p] != '"') {
            char c = s[p++];
            if (c != '\\') { out += c; continue; }
            if (p >= s.size()) Fail("bad escape", p);
            char e = s[p++];
            switch (e) {
                case '"': out += '"'; break;   case '\\': out += '\\'; break;
                case '/': out += '/'; break;   case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;  case 'n': out += '\n'; break;
                case 'r': out += '\r'; break;  case 't': out += '\t'; break;
                case 'u': {
                    if (p + 4 > s.size()) Fail("bad \\u escape", p);
                    unsigned cp = static_cast<unsigned>(std::strtoul(s.substr(p, 4).c_str(), nullptr, 16));
                    p += 4;
                    if (cp < 0x80) out += static_cast<char>(cp);
                    else if (cp < 0x800) { out += static_cast<char>(0xC0 | (cp >> 6)); out += static_cast<char>(0x80 | (cp & 0x3F)); }
                    else { out += static_cast<char>(0xE0 | (cp >> 12)); out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F)); out += static_cast<char>(0x80 | (cp & 0x3F)); }
                    break;
                }
                default: Fail("unknown escape", p);
            }
        }
        if (p >= s.size()) Fail("unterminated string", p);
        ++p;
        return out;
    }
    static basic_json ParseValue(const std::string& s, size_t& p) {
        SkipWs(s, p);
        if (p >= s.size()) Fail("unexpected end of input", p);
        basic_json v;
        char c = s[p];
        if (c == '{') {
            v.m_type = Type::Object; ++p; SkipWs(s, p);
            if (p < s.size() && s[p] == '}') { ++p; return v; }
            for (;;) {
                SkipWs(s, p);
                std::string key = ParseString(s, p);
                SkipWs(s, p);
                if (p >= s.size() || s[p] != ':') Fail("expected ':'", p);
                ++p;
                v.m_keys.push_back(key);
                v.m_items.push_back(ParseValue(s, p));
                SkipWs(s, p);
                if (p < s.size() && s[p] == ',') { ++p; continue; }
                if (p < s.size() && s[p] == '}') { ++p; break; }
                Fail("expected ',' or '}'", p);
            }
        } else if (c == '[') {
            v.m_type = Type::Array; ++p; SkipWs(s, p);
            if (p < s.size() && s[p] == ']') { ++p; return v; }
            for (;;) {
                v.m_items.push_back(ParseValue(s, p));
                SkipWs(s, p);
                if (p < s.size() && s[p] == ',') { ++p; continue; }
                if (p < s.size() && s[p] == ']') { ++p; break; }
                Fail("expected ',' or ']'", p);
            }
        } else if (c == '"') {
            v.m_type = Type::String; v.m_string = ParseString(s, p);
        } else if (s.compare(p, 4, "true") == 0) {
            v.m_type = Type::Bool; v.m_bool = true; p += 4;
        } else if (s.compare(p, 5, "false") == 0) {
            v.m_type = Type::Bool; v.m_bool = false; p += 5;
        } else if (s.compare(p, 4, "null") == 0) {
            p += 4;
        } else {
            const char* start = s.c_str() + p;
            char* end = nullptr;
            double d = std::strtod(start, &end);
            if (end == start) Fail("unexpected character", p);
            v.m_type = Type::Number; v.m_number = d; p += static_cast<size_t>(end - start);
        }
        return v;
    }
};

using json = basic_json;
}  // namespace nlohmann
