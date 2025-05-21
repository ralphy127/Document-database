#include "Database.hpp"

std::optional<std::reference_wrapper<Collection>> Database::getCollection(std::string collectionName) {
    auto it = _collections.find(std::move(collectionName));
    if(it != _collections.end()) {
        return std::ref(it->second);
    }

    return std::nullopt;
}

void Database::addCollection(std::string collectionName) {
    auto it = _collections.find(collectionName);
    if(it != _collections.end()) {
        Logger::logWarning(collectionName + " collection already exists.");
        return;
    }

    bool resetCollectionDirectory = true;
    std::string path = _name + '/' + collectionName;
    ensureDirectoryExists(path, resetCollectionDirectory);

    _collections.emplace(collectionName, Collection(std::move(collectionName)));
}

void Database::insertCollection(Collection collection) {
    std::string collectionName = collection.getName();
    std::string path = _name + '/' + collectionName;
    
    bool resetCollectionDirectory = true;
    ensureDirectoryExists(path, resetCollectionDirectory);

    for(auto doc : collection.getAll()) {
        _storage.saveDocument(path, doc);
    }

    _collections.insert({collectionName, std::move(collection)});
}

void Database::insert(std::string collectionName, Document doc) {
    auto it = _collections.find(collectionName);
    if(it == _collections.end()) {
        Logger::logWarning(collectionName + " does not exisst.");
        return;
    }

    std::string path = _name + '/' + collectionName;
    _storage.saveDocument(path, doc);
    auto& collection = it->second;
    collection.insert(std::move(doc));
}

void Database::insertVectorToDocument(std::string collectionName, std::vector<Document>& docs, std::string name, Document& doc) {
    auto it = _collections.find(collectionName);
    if(it == _collections.end()) {
        Logger::logWarning(collectionName + " does not exisst.");
        return;
    }
    
    doc.set(name, std::move(docs));
    std::string path = _name + '/' + collectionName;
    _storage.saveDocument(path, doc);
    it->second.insert(doc);
}

std::vector<Document> Database::getAll(std::string collectionName) const {
    auto it = _collections.find(collectionName);
    if(it == _collections.end()) {
        Logger::logWarning(collectionName + " does not exisst.");
        return std::vector<Document>();
    }

    auto& collection = it->second;

    return collection.getAll();
}

std::optional<Collection> Database::getCollection(std::string collectionName) const {
    auto it = _collections.find(collectionName);
    if(it != _collections.end()) {
        Logger::logWarning(collectionName + " collection already exists.");
        return std::nullopt;
    }

    return it->second;
}

void Database::ensureDirectoryExists(const std::filesystem::path& path, bool reset) {
    try {
        if (reset && std::filesystem::exists(path)) {
            std::filesystem::remove_all(path);
        }

        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directories(path);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::logError(e.what());
    }
}