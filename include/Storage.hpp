#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "Document.hpp"
#include "Logger.hpp"

/// @brief Represents storage providing saving and loading database files
class Storage {
public:
    /// @brief Load all documents in collection
    /// @param collectionPath Collection's path to load
    /// @return Documents
    std::vector<Document> loadDocuments(const std::string& collectionPath);

    /// @brief Save document in collection
    /// @param collectionPath Collection's path to save document
    /// @param doc Document to be saved
    void saveDocument(std::string collectionPath, const Document& doc);

    /// @brief Remove document from collection
    /// @param path Collection's path
    /// @param id Document's id to be removed
    void removeDocument(const std::filesystem::path& path, size_t id);

private:
    /// @brief Write tabs
    /// @param file File to write
    /// @param amount Amount of tabs to write
    void saveTabs(std::ofstream& file, size_t amount);

    /// @brief Save document in collection (implementation)
    /// @param collectionPath Collection's path
    /// @param doc Document to be saved
    /// @param tabs Number of tabs to start a line
    void saveDocument(std::string collectionPath, const Document& doc, size_t tabs);

    /// @brief Save single document
    /// @param doc Document to save
    /// @param tabs Number of tabs to start a line
    /// @param file File to save document
    void saveSingleDocument(const Document& doc, size_t tabs, std::ofstream& file);

    /// @brief Parse single document
    /// @param file Input document's file
    /// @return Read document
    Document parseDocument(std::ifstream& file);

     /// @brief Parse single vector
    /// @param file Input document's file
    /// @return Read vector
    Document::Vector parseVector(std::ifstream& file);

     /// @brief Parse single map
    /// @param file Input document's file
    /// @return Read map
    Document::Map parseMap(std::ifstream& file);

    /// @brief Remove leading and trailing whitespaces from a string
    /// @param source String to be trimmed
    /// @return New trimmed string
    std::string trim(const std::string& source);

    /// @brief Extract key from trimmed line
    /// @param trimmed Trimmed string to extract key
    /// @return New string representing key
    std::string parseKey(const std::string& trimmed);

    /// @brief Extract type from trimmed line
    /// @param trimmed Trimmed string to extract type
    /// @return New string representing type
    std::string parseType(const std::string& trimmed);

    /// @brief Extract value from trimmed line
    /// @param trimmed Trimmed string to extract value
    /// @return New string representing value
    std::string parseValue(const std::string& trimmed);

    /// @brief Extract special character from trimmed line
    /// @param trimmed Trimmed string to extract special character
    /// @return New characted representing special character
    char parseSpecial(const std::string& trimmed);

    /// @brief Check if a line is start of Document
    /// @param trimmed Rrimmed line
    /// @param key Key extracted from line
    /// @param type Type extracted from line
    /// @return True if it is start, false otherwise
    bool isDocumentStart(const std::string& trimmed, std::string& key, const std::string& type);

    /// @brief Check if a line is start of Document::Vector
    /// @param trimmed Rrimmed line
    /// @param key Key extracted from line
    /// @param type Type extracted from line
    /// @return True if it is start, false otherwise
    bool isVectorStart(const std::string& trimmed, const std::string& key, const std::string& type);

    /// @brief Check if a line is start of Document::Map
    /// @param trimmed Rrimmed line
    /// @param key Key extracted from line
    /// @param type Type extracted from line
    /// @return True if it is start, false otherwise
    bool isMapStart(const std::string& trimmed, const std::string& key, const std::string& type);
};