#include "Collection.hpp"
#include "Storage.hpp"

class Database {
public:
    Database(std::string name) : _name(std::move(name)) {}
    
    std::optional<std::reference_wrapper<Collection>> getCollection(std::string collectionName);

    void addCollection(std::string collectionName);

    void insertCollection(Collection collection);

    void insert(std::string collectionName, Document doc);

    template<typename Filter, typename Modifier>
    void update(std::string collectionName, Filter&& filter, Modifier&& modify);

    template<typename Filter>
    std::vector<Document> find(std::string collectionName, Filter&& filter);

    template<typename Filter>
    void remove(std::string collectionName, Filter&& filter);

    void insertVectorToDocument(std::string collectionName, std::vector<Document>& docs, std::string name, Document& doc);

    std::vector<Document> getAll(std::string collectionName) const;

    std::optional<Collection> getCollection(std::string collectionName) const;
private:
    std::string _name;
    std::unordered_map<std::string, Collection> _collections;
    Storage _storage;

    void ensureDirectoryExists(const std::filesystem::path& path, bool reset = false);
};






template<typename Filter, typename Modifier>
void Database::update(std::string collectionName, Filter&& filter, Modifier&& modify) {
    auto it = _collections.find(collectionName);
    if(it == _collections.end()) {
        Logger::logWarning("Tried to update documents in non exisitng collection of name: " + collectionName);
        return;
    }

    auto& collection = it->second;
    auto idsUpdated = collection.update(std::forward<Filter>(filter), std::forward<Modifier>(modify));

    std::string path = _name + '/' + collectionName;
    for(const auto& id : idsUpdated) {
        auto docOpt = collection.getDocumentById(id);
        if(docOpt) {
            _storage.saveDocument(path, *docOpt);
        }
    }
}

template<typename Filter>
std::vector<Document> Database::find(std::string collectionName, Filter&& filter) {
    auto it = _collections.find(collectionName);
    if(it == _collections.end()) {
        Logger::logWarning("Tried to find documents in non exisitng collection of name: " + collectionName);
        return std::vector<Document>();
    }

    auto& collection = it->second;
    return collection.find(std::forward<Filter>(filter));
}

template<typename Filter>
void Database::remove(std::string collectionName, Filter&& filter) {
    auto it = _collections.find(collectionName);
    if(it == _collections.end()) {
        Logger::logWarning("Tried to remove documents in non exisitng collection of name: " + collectionName);
        return;
    }

    auto& collection = it->second;

    auto docIds = collection.remove(std::forward<Filter>(filter));

    std::string path = _name + '/' + collectionName;
    for(const auto id : docIds) {
        _storage.removeDocument(path, id);
    }
}