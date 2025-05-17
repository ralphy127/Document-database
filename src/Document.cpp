#include "Document.hpp"

template<typename T>
void printValue(const T& value) {
    if constexpr(std::is_same_v<T, int> ||
                 std::is_same_v<T, size_t> ||
                 std::is_same_v<T, double> ||
                 std::is_same_v<T, std::string>) {
        std::cout << value;
    } 
    else if constexpr(std::is_same_v<T, bool>) {
        std::cout << (value ? "true" : "false");
    } 
    else if constexpr(std::is_same_v<T, std::vector<Document>>) {
        std::cout << "[\n";
        for(const auto& doc : value) {
            doc.printInfo();
            std::cout << ",\n";
        }
        std::cout << "]";
    }
    else if constexpr(std::is_same_v<T, std::unordered_map<std::string, Document>>) {
        std::cout << "{\n";
        for(const auto& [k, doc] : value) {
            std::cout << k << ": ";
            doc.printInfo();
            std::cout << ",\n";
        }
        std::cout << "}";
    } 
    else {
        std::cout << "<unsupported type>";
    }
}

void Document::printInfo() const {
    for(const auto& [key, val] : _data) {
        std::cout << key << ": ";
        std::visit([](const auto& v) {
            printValue(v);
        }, val);
        std::cout << "\n";
    }
}