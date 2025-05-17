#pragma once

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <stdexcept>
#include <type_traits>

class Document {
public:
    using Vector = std::vector<Document>;
    using Map = std::unordered_map<std::string, Document>;
    using Key = std::string;
    using Value = std::variant<int, size_t, double, std::string, bool, Vector, Map>;

    template<typename T>
    void set(const Key& key, const T& value);

    template<typename T>
    std::optional<T> get(const Key& key) const;

    void printInfo() const;

    bool hasField(std::string_view key) const { return _data.find(std::string(key)) != _data.end(); }

private:
    std::unordered_map<Key, Value> _data;

    template<typename T>
    static constexpr bool is_valid_type();
};





template<typename T>
void Document::set(const Key& key, const T& value) {
    static_assert(is_valid_type<T>(), "Invalid type for Document");
    _data[key] = value;
}

template<typename T>
std::optional<T> Document::get(const Key& key) const {
    auto it = _data.find(key);
    if (it != _data.end()) {
        if (auto val = std::get_if<T>(&it->second)) {
            return *val;
        }
    }
    
    return std::nullopt;
}

template<typename T>
constexpr bool Document::is_valid_type() {
    return
        std::is_same_v<T, int> ||
        std::is_same_v<T, size_t> ||
        std::is_same_v<T, double> ||
        std::is_same_v<T, std::string> ||
        std::is_same_v<T, bool> ||
        std::is_same_v<T, Vector> ||
        std::is_same_v<T, Map>;
}