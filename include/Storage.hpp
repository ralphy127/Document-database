#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "Document.hpp"
#include "Logger.hpp"

class Storage {
public:
    std::vector<Document> loadDocuments(std::string collectionPath) {
        std::string xd = collectionPath;
        return std::vector<Document>();
    }

    void saveDocument(std::string collectionPath, const Document& doc, size_t tabs = 0);

    void removeDocument(const std::filesystem::path& path, size_t id);
private:
    void saveTabs(std::ofstream& file, size_t amount);
    void saveSingleDocument(const Document& doc, size_t tabs, std::ofstream& file);
};