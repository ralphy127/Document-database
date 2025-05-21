#include "Storage.hpp"
#include <fstream>
#include <stdexcept>
#include <variant>
#include <type_traits>

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
            for(const auto& nestedDoc : *vectorType) {
                saveSingleDocument(nestedDoc, tabs + 2, file);
                file << '\n';
            }
            saveTabs(file, tabs + 1);
            file << "]\n";
        }
        else if(const auto* mapType = std::get_if<Document::Map>(&val)) {
            saveTabs(file, tabs + 1);
            file << key << " (Document::Map>) : {\n";
            for(const auto& [subkey, subdoc] : *mapType) {
                saveTabs(file, tabs + 2);
                file << subkey << " : ";
                saveSingleDocument(subdoc, tabs + 2, file);
                file << '\n';
            }
            saveTabs(file, tabs + 1);
            file << "}\n";
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
            throw std::runtime_error("Trying to save document with wrong wariant type in document of id: " + std::to_string(*id) + '.');
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
            Logger::logInfo("Deleted document file: " + filePath.string());
        } 
        else {
            Logger::logWarning("Document file not found: " + filePath.string());
        }
    }
    catch(const std::filesystem::filesystem_error& e) {
        Logger::logError("Filesystem error while deleting document: " + std::string(e.what()));
    }
}

void Storage::saveTabs(std::ofstream& file, size_t amount) {
    for(size_t i{0}; i < amount; ++i) {
        file << '\t';
    }
}