#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "Document.hpp"
#include "Logger.hpp"

class Storage {
public:
    Storage(std::string collectionPath);
    
    std::vector<Document> loadDocuments() {
        return std::vector<Document>();
    }

    void saveDocument(const Document& doc, size_t tabs = 0);

private:
    std::string _collectionPath;

    void saveTabs(std::ofstream& file, size_t amount);
    void ensureDirectoryExists(const std::filesystem::path& path);
    void removeDirectory(const std::filesystem::path& dirPath); 
    void saveSingleDocument(const Document& doc, size_t tabs, std::ofstream& file);
};