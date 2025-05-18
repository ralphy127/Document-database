#include "Collection.hpp"

void Collection::insert(Document doc) {
    try {
        auto id = generateId();
        doc.set("id", id);

        auto pos = _documents.size();

        for(auto& [field, positions] : _indexes) {
            if(doc.hasField(field)) {
                positions.push_back(pos);
            }
        }
        
        _ids.insert(id);
        _documents.push_back(std::move(doc));

        Logger::logInfo("Added document of id: " + std::to_string(id) + ".");
    }
    catch(const std::runtime_error& e) {
        Logger::logError(e.what());
    }
}

void Collection::insertVectorToDocument(std::vector<Document>& docs, std::string name, Document& doc) {
    try {
        for(auto& d : docs) {
            auto id = generateId();
            d.set("id", id);
            _ids.insert(id);
        }

        doc.set(std::move(name), std::move(docs));
        
        if(auto id = doc.get<size_t>("id")) {
            Logger::logInfo("Added vector of " + std::to_string(docs.size()) + " documents to document of id: " + std::to_string(*id) + '.');
        }
        else {
            Logger::logWarning("Added vector of " + std::to_string(docs.size()) + " documents to document of unknown id.");
        }
    }
    catch(const std::runtime_error& e) {
        Logger::logError(e.what());
    } 
}

size_t Collection::generateId() {
    auto maxIterations{100};
    for(auto i = 0; i < maxIterations; ++i) {
        auto id = _rng();
        if(_ids.find(id) == _ids.end()) {
            return id;
        }
    }

    throw std::runtime_error("Failed to generate unique document ID after " + std::to_string(maxIterations) + " attempts.");
}