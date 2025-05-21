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

        Logger::logInfo("Added document of id: " + std::to_string(id) + " in collection: " + _name + '.');
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
            Logger::logInfo("Added vector of " + std::to_string(docs.size()) + " documents to document of id: " + std::to_string(*id) + " in collection: " + _name + '.');
        }
        else {
            Logger::logWarning("Added vector of " + std::to_string(docs.size()) + " documents to document of unknown id in collection: " + _name + '.');
        }
    }
    catch(const std::runtime_error& e) {
        Logger::logError(e.what());
    } 
}

void Collection::update(Document& newDoc) {
    auto idOpt = newDoc.get<size_t>("id");
    if (!idOpt) {
        Logger::logWarning("Tried to update a document without id in collection: " + _name + '.');
        return;
    }

    size_t id = *idOpt;

    for (size_t pos = 0; pos < _documents.size(); ++pos) {
        auto& currentDoc = _documents[pos];

        auto currentIdOpt = currentDoc.get<size_t>("id");
        if (!currentIdOpt || *currentIdOpt != id) {
            continue;
        }

        std::unordered_set<std::string> hadFields;
        for (const auto& [field, _] : _indexes) {
            if (currentDoc.hasField(field)) {
                hadFields.insert(field);
            }
        }

        currentDoc = newDoc;

        for (auto& [field, positions] : _indexes) {
            bool had = hadFields.count(field);
            bool hasNow = currentDoc.hasField(field);

            auto it = std::find(positions.begin(), positions.end(), pos);

            if (had && !hasNow && it != positions.end()) {
                positions.erase(it);
            } 
            else if (!had && hasNow && it == positions.end()) {
                positions.push_back(pos);
            }
        }

        Logger::logInfo("Updated document of id: " + std::to_string(id) + " in collection: " + _name + '.');
        return;
    }

    Logger::logWarning("No document with id " + std::to_string(id) + " found to update in collection: " + _name + '.');
}

void Collection::remove(Document& doc) {
    auto idOpt = doc.get<size_t>("id");
    if (!idOpt) {
        Logger::logWarning("Tried to remove document without id in collection: " + _name + '.');
        return;
    }

    auto id = *idOpt;
    _ids.erase(id);

    auto it = std::find_if(_documents.begin(), _documents.end(),
        [id](const Document& d) {
            auto dId = d.get<size_t>("id");
            return dId && *dId == id;
        });

    if (it == _documents.end()) {
        Logger::logWarning("Tried to remove non-existing document of id: " + std::to_string(id) + " in collection:" + _name + '.');
        return;
    }
    
    size_t index = std::distance(_documents.begin(), it);
    _documents.erase(it);

    for (auto& [field, positions] : _indexes) {
        positions.erase(std::remove(positions.begin(), positions.end(), index), positions.end());
        for (auto& pos : positions) {
            if (pos > index) {
                --pos;
            }
        }
    }

    Logger::logInfo("Removed document of id: " + std::to_string(id) + " in collection: " + _name + '.');
}

std::optional<Document> Collection::getDocumentById(size_t id) {
    for(const auto& doc : _documents) {
        auto idOpt = doc.get<size_t>("id");
        if(idOpt && *idOpt == id) {
            return doc;
        }
    }

    return std::nullopt;
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