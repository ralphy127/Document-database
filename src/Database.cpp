#include "Database.hpp"

Database::Database(std::string path) : _path(std::move(path)) {
    _name = _path.substr(_path.find_last_of("/") + 1);

    ensureDirectoryExists(static_cast<std::filesystem::path>(_path));

    for(const auto& entry : std::filesystem::directory_iterator(_path)) {
        auto& collectionPath = entry.path();

        if(collectionPath == (_path + "/.DS_Store")) {
            continue;
        }

        auto collectionPathString = collectionPath.string();
        auto collectionName = collectionPathString.erase(0, collectionPathString.find_last_of("/") + 1);

        try {
            Collection collection(collectionName);
            for(auto& doc : _storage.loadDocuments(collectionPath)) {
                collection.insert(doc);
            }

            auto [iter, inserted] = _collections.try_emplace(collectionName, std::move(collection));

            if (!inserted) {
                throw std::runtime_error("Collection '" + collectionName + "' already exists.");
            }
        } catch (const std::exception& e) {
            Logger::logError("Failed to load collection '" + collectionName + "': " + e.what() + ".");
            throw;
        }
    }

    Logger::logInfo("Succesfully loaded database: " + _path + ".");
}

std::optional<std::reference_wrapper<Collection>> Database::getCollection(std::string collectionName) {
    auto it = _collections.find(std::move(collectionName));
    if(it != _collections.end()) {
        return std::ref(it->second);
    }

    return std::nullopt;
}

std::optional<Collection> Database::getCollectionCopy(std::string collectionName) const {
    auto it = _collections.find(collectionName);
    if(it == _collections.end()) {
        Logger::logWarning(collectionName + "  does not exist in database: " + _name + ".");
        return std::nullopt;
    }

    return it->second;
}

void Database::addCollection(std::string collectionName) {
    auto it = _collections.find(collectionName);
    if(it != _collections.end()) {
        Logger::logWarning(collectionName + " collection already exists in database: " + _name + ".");
        return;
    }

    bool resetCollectionDirectory = true;

    std::string path = _path + '/' + collectionName;
    ensureDirectoryExists(path, resetCollectionDirectory);

    _collections.emplace(collectionName, Collection(collectionName));
}

void Database::insertCollection(Collection collection) {
    std::string collectionName = collection.getName();

    if(_collections.find(collectionName) != _collections.end()) {
        Logger::logWarning(collectionName + " already exists in database: " + _name + ".");
        return;
    }

    std::string path = _path + '/' + collectionName;
    
    bool resetCollectionDirectory = true;
    ensureDirectoryExists(path, resetCollectionDirectory);

    for(auto doc : collection.getAll()) {
        _storage.saveDocument(path, doc);
    }

    _collections.insert({collectionName, std::move(collection)});
    Logger::logInfo("Inserted new collection: " + collectionName + " to database: " + _name + ".");
}

void Database::insert(std::string collectionName, Document doc) {
    auto it = _collections.find(collectionName);
    if(it == _collections.end()) {
        Logger::logWarning(collectionName + " does not exist in database: " + _name + ".");
        return;
    }   

    auto& collection = it->second;
    collection.insert(doc);

    std::string path = _path + '/' + collectionName;
    _storage.saveDocument(path, doc);
}

void Database::remove(std::string collectionName, Document& doc) {
    auto it = _collections.find(collectionName);
    if(it == _collections.end()) {
        Logger::logWarning(collectionName + " does not exist in database: " + _name + ".");
        return;
    }
    auto idOpt = doc.get<size_t>("id");
    if(!idOpt) {
        Logger::logWarning("Tried to remove document without id.");
        return;
    }

    auto& collection = it->second;
    collection.remove(doc);

    std::string colllectionPath = _path + "/" + collectionName;
    _storage.removeDocument(colllectionPath, *idOpt);
}

std::vector<Document> Database::getAll(std::string collectionName) const {
    auto it = _collections.find(collectionName);
    if(it == _collections.end()) {
        Logger::logWarning(collectionName + " does not exist in database: " + _name + ".");
        return std::vector<Document>();
    }

    auto& collection = it->second;

    return collection.getAll();
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