#pragma once

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <stdexcept>
#include <type_traits>

/// @brief Represents single document
class Document {
public:
    /// @brief Represents vector of documents
    using Vector = std::vector<Document>;

    /// @brief Represents map: key - document's name, value - document
    using Map = std::unordered_map<std::string, Document>;

    /// @brief Represents values which document is able to store
    using Value = std::variant<int, size_t, double, std::string, bool, Document, Vector, Map>;

    /// @brief Add and set document's property
    /// @tparam T Property's typename
    /// @param key Name of property
    /// @param value Value of property
    template<typename T>
    void set(const std::string& key, const T& value);

    /// @brief Get copy of document's property
    /// @tparam T Property's typename
    /// @param key Name of property
    /// @return Copy of property's value
    template<typename T>
    std::optional<T> get(const std::string& key) const;

    /// @brief Check if field exists
    /// @param key Name of property
    /// @return True if field exists, false otherwise   
    bool hasField(const std::string& key) const { return _data.find(key) != _data.end(); }

    /// @brief Remove data from document
    /// @param key Data key to be removed
    void remove(const std::string& key) { _data.erase(key); };

    /// @brief Get document's data view
    /// @return Constant map of properties
    const std::unordered_map<std::string, Value>& getDataView() const { return _data; }

    /// @brief Get document's data
    /// @return Reference to map of properties
    std::unordered_map<std::string, Value>& getData() { return _data; }

    /// @brief Overloaded operator ==
    friend bool operator==(const Document& lhs, const Document& rhs) { return lhs._data == rhs._data; }

    /// @brief Overloaded operator !=
    friend bool operator!=(const Document& lhs, const Document& rhs) { return !(lhs == rhs); }
private:
    /// @brief Data stored by document
    std::unordered_map<std::string, Value> _data;

    /// @brief Check if type is valid
    /// @tparam T 
    /// @return True if T is one of variants of Document::Value, false otherwise
    template<typename T>
    static constexpr bool is_valid_type();
};





template<typename T>
void Document::set(const std::string& key, const T& value) {
    static_assert(is_valid_type<T>(), "Invalid type for Document");

    if (key == "id" && !std::is_same_v<T, size_t>) {
        throw std::invalid_argument("Field 'id' must be of type size_t");
    }

    _data[key] = value;
}

template<typename T>
std::optional<T> Document::get(const std::string& key) const {
    auto it = _data.find(key);
    if(it != _data.end()) {
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
        std::is_same_v<T, Document> ||
        std::is_same_v<std::decay_t<T>, Vector> ||
        std::is_same_v<std::decay_t<T>, Map>;
}