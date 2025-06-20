#include "Storage.hpp"

#include <fstream>
#include <stdexcept>
#include <variant>
#include <type_traits>
#include <fstream>
#include <string>

void Storage::saveDocument(std::string collectionPath, const Document& doc) {
    saveDocument(collectionPath, doc, 0);
}

void Storage::saveDocument(std::string collectionPath, const Document& doc, size_t tabs) {
    
    auto idOpt = doc.get<size_t>("id");
    if(!idOpt) {
        throw std::runtime_error("Trying to save document without id.");
    }

    auto id = *idOpt;
    std::ofstream file(collectionPath + '/' + std::to_string(id) + ".txt");
    if(!file.is_open()) {
        throw std::runtime_error("Cannot open a file to save document of id: " + std::to_string(id));
    }

    saveSingleDocument(doc, tabs, file);

    file.close();
}

void Storage::saveSingleDocument(const Document& doc, size_t tabs, std::ofstream& file) {
    saveTabs(file, tabs);
    file << "{\n";

    for(const auto& pair : doc.getDataView()) {
        const auto& key = pair.first;
        const auto& val = pair.second;

        if(const auto* vectorType = std::get_if<Document::Vector>(&val)) {
            saveTabs(file, tabs + 1);
            file << key << " (Document::Vector) : [\n";
            for(size_t i{0}; i < (*vectorType).size(); ++i) {
                saveTabs(file, tabs + 2);
                file << '[' << i << "]\n";
                saveSingleDocument((*vectorType)[i], tabs + 2, file);
                file << '\n';
            }
            saveTabs(file, tabs + 1);
            file << "]";
        }
        else if(const auto* mapType = std::get_if<Document::Map>(&val)) {
            saveTabs(file, tabs + 1);
            file << key << " (Document::Map) : {\n";
            for(const auto& [subkey, subdoc] : *mapType) {
                saveTabs(file, tabs + 2);
                file << subkey << " : \n";
                saveSingleDocument(subdoc, tabs + 2, file);
                file << '\n';
            }
            saveTabs(file, tabs + 1);
            file << "}";
        }
        else if(const auto* documentType = std::get_if<Document>(&val)) {
            saveTabs(file, tabs + 1);
            file << key << " (Document)\n";
            saveSingleDocument(*documentType, tabs + 1, file);
        }
        else if(const auto* boolType = std::get_if<bool>(&val)) {
            saveTabs(file, tabs + 1);
            file << key << " (bool) : " << (*boolType ? "true" : "false");
        }
        else if(const auto* intType = std::get_if<int>(&val)) {
            saveTabs(file, tabs + 1);
            file << key << " (int) : " << *intType;
        } 
        else if(const auto* doubleType = std::get_if<double>(&val)) {
            saveTabs(file, tabs + 1);
            file << key << " (double) : " << *doubleType;
        } 
        else if(const auto* size_tType = std::get_if<size_t>(&val)) {
            saveTabs(file, tabs + 1);
            file << key << " (size_t) : " << *size_tType;
        } 
        else if(const auto* stringType = std::get_if<std::string>(&val)) {
            saveTabs(file, tabs + 1);
            file << key << " (std::string) : " << *stringType;
        }
        else if(auto id = doc.get<size_t>("id")) {
            throw std::runtime_error("Trying to save document with wrong wariant type in document of id: " + std::to_string(*id) + ".");
        }
        else {
            throw std::runtime_error("Trying to save document with wrong wariant type in document of unknown id.");
        }
        file << '\n';    
    }

    saveTabs(file, tabs);
    file << "}";
}

void Storage::removeDocument(const std::filesystem::path& path, size_t id) {
    auto filePath = path / (std::to_string(id) + ".txt");

    try {
        if(std::filesystem::exists(filePath)) {
            std::filesystem::remove(filePath);
            Logger::logInfo("Deleted document file: " + filePath.string() + ".");
        } 
        else {
            Logger::logWarning("Document file not found: " + filePath.string() + ".");
        }
    }
    catch(const std::filesystem::filesystem_error& e) {
        Logger::logError("Filesystem error while deleting document: " + std::string(e.what()) + ".");
    }
}

void Storage::saveTabs(std::ofstream& file, size_t amount) {
    for(size_t i{0}; i < amount; ++i) {
        file << '\t';
    }
}

std::vector<Document> Storage::loadDocuments(const std::string& collectionPath) {
    std::vector<Document> documents;

    for(const auto& entry : std::filesystem::directory_iterator(collectionPath)) {
        if(!entry.is_regular_file() || entry.path().extension() != ".txt") {
            continue;
        }

        std::ifstream file(entry.path());

        if (!file.is_open()) {
            Logger::logWarning("Could not open file: " + entry.path().string());
            continue;
        }

        try {
            documents.push_back(parseDocument(file));
        } catch (const std::exception& e) {
            Logger::logError("Failed to parse document " + entry.path().string() + ": " + e.what());
        }
    }

    return documents;
}

std::string Storage::trim(const std::string& source) {
    std::string trimmed(source);
    trimmed.erase(0, trimmed.find_first_not_of(" \n\r\t"));
    trimmed.erase(trimmed.find_last_not_of(" \n\r\t") + 1);
    return trimmed;
}

std::string Storage::parseKey(const std::string& trimmed) {
    std::string key(trimmed);
    key.erase(key.find_first_of(" \n\r\t") + 1);
    return trim(key);
}

std::string Storage::parseType(const std::string& trimmed) {
    if(trimmed.empty()) {
        return std::string();
    }

    std::string type(trimmed);
    type.erase(0, type.find_first_of(" \n\r\t") + 1);

    switch(type[0]) {
        case '{':
        case '}':
        case '[':
        case ']':
        case ':':
            return std::string();
    }

    type.erase(0, 1);
    type.erase(type.find_first_of(")"));

    return type;
}

std::string Storage::parseValue(const std::string& trimmed) {
    std::string value(trimmed);
    value.erase(0, value.find_first_of(")") + 1);
    value.erase(0, value.find_first_of(":") + 1);
    value.erase(0, value.find_first_of(" ") + 1);
    return value;
}

char Storage::parseSpecial(const std::string& trimmed) {
    if(trimmed.empty()) {
        return '\0';
    }

    std::string special(trimmed);

    switch(special[0]) {
        case '{':
        case '}':
        case '[':
        case ']':
        case ':':
            return special[0];
        default:
            return '\0';
    }
}

bool Storage::isDocumentStart(const std::string& trimmed, std::string& key, const std::string& type) {
    return !trimmed.empty() && !key.empty() && type == "Document";
}

bool Storage::isVectorStart(const std::string& trimmed, const std::string& key, const std::string& type) {
    return !key.empty() && type == "Document::Vector" && trimmed[trimmed.size() - 1] == '[';
}

bool Storage::isMapStart(const std::string& trimmed, const std::string& key, const std::string& type) {
    return !key.empty() && type == "Document::Map" && trimmed[trimmed.size() - 1] == '{';
}

Document Storage::parseDocument(std::ifstream& file) {
    Document doc;
    std::string line;

    while (std::getline(file, line)) {
        auto trimmed = trim(line);

        if (trimmed.empty()) {
            continue;
        }

        if (trimmed == "}") {
            break;
        }

        auto key = parseKey(trimmed);
        auto type = parseType(trimmed);
        auto value = parseValue(trimmed);

        if (isDocumentStart(trimmed, key, type)) {
            auto nestedDoc = parseDocument(file);
            doc.set(key, std::move(nestedDoc));
        }
        else if (isVectorStart(trimmed, key, type)) {
            auto vector = parseVector(file);
            doc.set(key, std::move(vector));
        }
        else if (isMapStart(trimmed, key, type)) {
            auto map = parseMap(file);
            doc.set(key, std::move(map));
        }
        else if (!key.empty() && !type.empty()) {
            if (type == "bool") {
                doc.set(key, value == "true");
            }
            else if (type == "int") {
                doc.set(key, std::stoi(value));
            }
            else if (type == "double") {
                doc.set(key, std::stod(value));
            }
            else if (type == "size_t") {
                doc.set(key, static_cast<size_t>(std::stoull(value)));
            }
            else if (type == "std::string") {
                doc.set(key, value);
            }
        }
    }

    return doc;
}

Document::Vector Storage::parseVector(std::ifstream& file) {
    Document::Vector vector;
    std::string line;

    while (std::getline(file, line)) {
        auto trimmed = trim(line);

        if (trimmed.empty()) {
            continue;
        }

        if (trimmed == "]") {
            break;
        }

        if (trimmed[0] == '[' && trimmed.back() == ']') {
            continue;
        }

        if (trimmed == "{") {
            vector.emplace_back(parseDocument(file));
        }
    }

    return vector;
}

Document::Map Storage::parseMap(std::ifstream& file) {
    Document::Map map;
    std::string line;
    std::string current_key;

    while (std::getline(file, line)) {
        auto trimmed = trim(line);

        if (trimmed.empty()) {
            continue;
        }

        if (trimmed == "}") {
            break;
        }

        auto colon_pos = trimmed.find(':');
        if (colon_pos != std::string::npos) {
            current_key = trim(trimmed.substr(0, colon_pos));
            std::getline(file, line);
            trimmed = trim(line);
            if (trimmed == "{") {
                map[current_key] = parseDocument(file);
            }
        }
    }

    return map;
}