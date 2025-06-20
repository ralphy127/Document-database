#include "Collection.hpp"

void Collection::insert(Document& doc) {
    try {
        auto optId = doc.get<size_t>("id");
        if (optId && _ids.find(*optId) != _ids.end()) {
            Logger::logWarning("Document with id " + std::to_string(*optId) + " already exists in collection: " + _name + ".");
            return;
        }

        auto id = optId.value_or(generateId());
        doc.set("id", id);
        
        _ids.insert(id);
        fillDocumentWithIds(doc);
        _documents.push_back(doc);

        Logger::logInfo("Added document of id: " + std::to_string(id) + " in collection: " + _name + ".");
    }
    catch(const std::runtime_error& e) {
        Logger::logError(e.what());
    }
}

void Collection::update(Document& newDoc) {
    auto idOpt = newDoc.get<size_t>("id");
    if (!idOpt) {
        Logger::logWarning("Tried to update a document without id in collection: " + _name + ".");
        return;
    }

    size_t id = *idOpt;

    for (size_t pos = 0; pos < _documents.size(); ++pos) {
        auto& currentDoc = _documents[pos];

        auto currentIdOpt = currentDoc.get<size_t>("id");
        if (!currentIdOpt || *currentIdOpt != id) {
            continue;
        }

        currentDoc = newDoc;

        Logger::logInfo("Updated document of id: " + std::to_string(id) + " in collection: " + _name + ".");
        return;
    }

    Logger::logWarning("No document with id " + std::to_string(id) + " found to update in collection: " + _name + ".");
}

void Collection::remove(Document& doc) {
    auto idOpt = doc.get<size_t>("id");
    if (!idOpt) {
        Logger::logWarning("Tried to remove document without id in collection: " + _name + ".");
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
        Logger::logWarning("Tried to remove non-existing document of id: " + std::to_string(id) + " in collection:" + _name + ".");
        return;
    }

    _documents.erase(it);

    Logger::logInfo("Removed document of id: " + std::to_string(id) + " in collection: " + _name + ".");
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
    for(auto i{0}; i < maxIterations; ++i) {
        auto id = _rng();
        if(_ids.find(id) == _ids.end()) {
            return id;
        }
    }

    throw std::runtime_error("Failed to generate unique document ID after " + std::to_string(maxIterations) + " attempts.");
}

void Collection::fillDocumentWithIds(Document& doc) {
    if(!doc.hasField("id")) {
        doc.set("id", generateId());
    }

    for(auto& [key, value] : doc.getData()) {
        std::visit([&](auto& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr(std::is_same_v<T, Document>) {
                fillDocumentWithIds(val);
            } 
            else if constexpr(std::is_same_v<T, Document::Vector>) {
                fillContainerWithIds(val);
                for(auto& d : val) {
                    fillDocumentWithIds(d);
                }
            } 
            else if constexpr(std::is_same_v<T, Document::Map>) {
                fillContainerWithIds(val);
                for(auto& [_, d] : val) {
                    fillDocumentWithIds(d);
                }
            }
        }, value);
    }
}