#include "Collection.hpp"
#include "Storage.hpp"

/// @brief Represents a database containing named collections
class Database {
public:
    /// @brief Construct a database
    /// @param name Name of the database
    Database(std::string name) : _name(std::move(name)) {}
    
    /// @brief Get mutable reference to collection
    /// @param collectionName Name of collection
    /// @return Optional reference to collection if it exists
    std::optional<std::reference_wrapper<Collection>> getCollection(std::string collectionName);
    
    /// @brief Add empty collection to database
    /// @param collectionName Name of new collection
    void addCollection(std::string collectionName);
    
    /// @brief Insert existing collection into database
    /// @param collection Collection to insert
    void insertCollection(Collection collection);
    
    /// @brief Insert document into collection
    /// @param collectionName Name of collection
    /// @param doc Document to insert
    void insert(std::string collectionName, Document doc);
    
    /// @brief Update documents in collection matching filter
    /// @tparam Filter Function
    /// @tparam Modifier Function
    /// @param collectionName Name of collection
    /// @param filter Function filtering documents to update
    /// @param modify Function modifying filtered documents
    template<typename Filter, typename Modifier>
    void update(std::string collectionName, Filter&& filter, Modifier&& modify);
    
    /// @brief Find documents in collection matching filter
    /// @tparam Filter Function
    /// @param collectionName Name of collection
    /// @param filter Function filtering documents
    /// @return Vector of copies of matching documents
    template<typename Filter>
    std::vector<Document> find(std::string collectionName, Filter&& filter);
    
    /// @brief Remove documents in collection matching filter
    /// @tparam Filter Function
    /// @param collectionName Name of collection
    /// @param filter Function filtering documents to remove
    template<typename Filter>
    void remove(std::string collectionName, Filter&& filter);
    
    /// @brief Insert container into a document within a collection
    /// @tparam Container Document::Map or Document::Vector
    /// @param collectionName Name of collection
    /// @param container Container to insert
    /// @param name Name of container
    /// @param doc Document to receive container
    template<typename Container>
    void insertContainerToDocument(std::string collectionName, Container& container, std::string name, Document& doc);
    
    /// @brief Get all documents from collection
    /// @param collectionName Name of collection
    /// @return Vector of copies of all documents
    std::vector<Document> getAll(std::string collectionName) const;
    
    /// @brief Get const copy of collection if exists
    /// @param collectionName Name of collection
    /// @return Optional collection copy if exists
    std::optional<Collection> getCollection(std::string collectionName) const;
    
private:
    /// @brief Database name
    std::string _name;
    
    /// @brief Map of collection names to collections
    std::unordered_map<std::string, Collection> _collections;
    
    /// @brief Object responsible for storing data
    Storage _storage;
    
    /// @brief Ensure a directory exists on filesystem
    /// @param path Path to check
    /// @param reset If true, clears directory if exists
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

template<typename Container>
void Database::insertContainerToDocument(std::string collectionName, Container& container, std::string name, Document& doc) {
    auto it = _collections.find(collectionName);
    if(it == _collections.end()) {
        Logger::logWarning(collectionName + " does not exisst.");
        return;
    }

    auto& collection = it->second;

    collection.fillContainerWithIds(container);

    doc.set(name, container);
    std::string path = _name + '/' + collectionName;
    _storage.saveDocument(path, doc);

    auto id = doc.get<size_t>("id").value_or(0);
    if(id != 0 && collection.getDocumentById(id)) {
        collection.update(doc);
    }
    else {
        collection.insert(doc);
    }
}