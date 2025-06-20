#include "Document.hpp"
#include "Logger.hpp"

#include <random>
#include <unordered_set>

/// @brief Represents a collection with documents
class Collection {
public:
    /// @brief Construct a collection
    /// @param name Name of collection
    Collection(std::string name) : _name(std::move(name)) {}

    /// @brief Insert document
    /// @param doc Document to be inserted
    void insert(Document& doc);

    /// @brief Update documents
    /// @tparam Filter Function
    /// @tparam Modifier Function
    /// @param filter Function filtering which documents should be updated
    /// @param modify Functions which modifies all documents found by filter
    /// @return Vector of indexes updated
    template<typename Filter, typename Modifier>
    std::vector<size_t> update(Filter&& filter, Modifier&& modify);

    /// @brief Update document
    /// @param doc Document to be updated
    void update(Document& doc);

    /// @brief Find dicuments
    /// @tparam Filter Function
    /// @param filter Function filtering documents
    /// @return Vector of copies of found documents
    template<typename Filter>
    std::vector<Document> find(Filter&& filter);

    /// @brief Remove documents
    /// @tparam Filter Function
    /// @param filter Function filtering which documents should be updated
    /// @return Vector of indexes removed
    template<typename Filter>
    std::vector<size_t> remove(Filter&& filter);

    /// @brief Remove document
    /// @param doc Document
    void remove(Document& doc);

    /// @brief Insert container to document
    /// @tparam Container Document::Map or Document::Vector
    /// @param container Container to be added
    /// @param name Name of container
    /// @param doc Document to which container will be added, if it does not exist in collection, create it
    template<typename Container>
    void insertContainerToDocument(Container& container, std::string name, Document& doc);

    /// @brief Get all documents
    /// @return Copy of all documents
    std::vector<Document> getAll() const { return _documents; }

    /// @brief Fil container with collection's unique ids
    /// @tparam Container Document::Map or Document::Vector
    /// @param container Container which documents to be filled
    template<typename Container>
    void fillContainerWithIds(Container& container);

    /// @brief Get name
    /// @return Copy of collection's name
    std::string getName() const { return _name; }

    /// @brief Get document by id
    /// @param id Document's id
    /// @return Document if one exists, std::nullopt otherwise
    std::optional<Document> getDocumentById(size_t id);

private:
    /// @brief Collection's name
    std::string _name;

    /// @brief Documents in collection
    std::vector<Document> _documents;

    /// @brief Set of documents id
    std::unordered_set<size_t> _ids;

    /// @brief Random number generator
    std::mt19937_64 _rng{std::random_device{}()};

    /// @brief Generate unique id
    /// @return Id
    size_t generateId();

    /// @brief Fill document with unique ids
    /// @param document Document which nested documents to be filled
    void fillDocumentWithIds(Document& document);

    /// @brief Check if Filter is of format: bool **Filter**(const Document&)
    /// @tparam Filter 
    template<typename Filter>
    static constexpr void assert_filter() { static_assert(std::is_invocable_r_v<bool, Filter, const Document&>, 
        "Filter must be callable with const Document& and return bool"); }

    /// @brief Check if Modifier is of format: void **Modifier**(Document&)
    /// @tparam Modifier 
    template<typename Modifier>
    static constexpr void assert_modifier() { static_assert(std::is_invocable_r_v<void, Modifier, Document&>,
        "Modifier must be callable with Document& and return void."); }
};





template<typename Filter, typename Modifier>
std::vector<size_t> Collection::update(Filter&& filter, Modifier&& modify) {
    assert_filter<Filter>();
    assert_modifier<Modifier>();

    std::vector<size_t> idsUpdated;

    for(size_t pos{0}; pos < _documents.size(); ++pos) {
        auto& doc = _documents[pos];

        if(filter(doc)) {
            modify(doc);

            auto idOpt = doc.get<size_t>("id");
            if (idOpt) {
                idsUpdated.push_back(*idOpt);
                Logger::logInfo("Modified document of id: " + std::to_string(static_cast<size_t>(*idOpt)) + " in collection: " + _name + ".");
            } else {
                Logger::logWarning("Modified document with no id in collection: " + _name + ".");
            }
        }
    }

    return idsUpdated;
}

template<typename Filter>
std::vector<Document> Collection::find(Filter&& filter) {
    assert_filter<Filter>();

    std::vector<Document> results;
    for(const auto& doc : _documents) {
        if(filter(doc)) {
            results.push_back(doc);
        }
    }

    return results;
}

template<typename Filter>
std::vector<size_t> Collection::remove(Filter&& filter) {
    assert_filter<Filter>();

    std::vector<size_t> toRemove;
    for(size_t i{0}; i < _documents.size(); ++i) {
        if(filter(_documents[i])) {
            toRemove.push_back(i);
        }
    }

    std::vector<size_t> docIds;

    if(toRemove.empty()) {
        Logger::logWarning("Tried to remove non existing document in collection" + _name + ".");
    }

    for(auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
        size_t i{*it};
        auto id = _documents[i].get<size_t>("id").value_or(0);
        docIds.push_back(id);

        _ids.erase(id);

        _documents.erase(_documents.begin() + i);
        Logger::logInfo("Removed document of id: " + std::to_string(id) + "in collection: " + _name + ".");
    }

    return docIds;
}

template<typename Container>
void Collection::insertContainerToDocument(Container& container, std::string name, Document& doc) {
    auto idOpt = doc.get<size_t>("id");
    if(!idOpt.has_value()) {
        size_t id = generateId();
        doc.set("id", id);
        idOpt = id;
    }

    size_t id = *idOpt;
    fillContainerWithIds(container);

    try {
        doc.set(name, container);
    } 
    catch (const std::exception& e) {
        Logger::logError("Failed to insert container into document with id: " + std::to_string(id) + " in collection: " + _name + ": " + e.what());
        return;
    }

    auto it = std::find_if(_documents.begin(), _documents.end(), [&](const Document& d) {
        auto existingId = d.get<size_t>("id");
        return existingId && *existingId == id;
    });

    if(it != _documents.end()) {
        *it = doc;
        Logger::logInfo("Updated existing document with id: " + std::to_string(id) + " in collection: " + _name + ".");
    } 
    else {
        _documents.push_back(doc);
        _ids.insert(id);
        Logger::logInfo("Inserted new document with id: " + std::to_string(id) + " in collection: " + _name + ".");
    }
}


template<typename Container>
void Collection::fillContainerWithIds(Container& container) {
    if constexpr(std::is_same_v<std::decay_t<Container>, Document::Vector>) {
        try {
            for(auto& doc : container) {
                if(doc.template get<size_t>("id")) {
                    continue;
                }

                auto id = generateId();
                doc.set("id", id);
            }
        }
        catch(std::runtime_error e) {
            Logger::logError("Failed to add Document::Vector to collection " + _name + ": " + e.what());
            return;
        }
    }
    else if constexpr(std::is_same_v<std::decay_t<Container>, Document::Map>) {
        try {
            for(auto& [name, doc] : container) {
                if(doc.template get<size_t>("id")) {
                    continue;
                }

                auto id = generateId();
                doc.set("id", id);
            }
        }
        catch(std::runtime_error e) {
            Logger::logError("Failed to add Document::Map to collection " + _name + ": " + e.what());
            return;
        }
    }
}