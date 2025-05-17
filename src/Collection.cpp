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
        
        _ids[id] = pos;
        _documents.push_back(std::move(doc));

        Logger::logInfo("Added document of id: " + std::to_string(id) + ".");
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