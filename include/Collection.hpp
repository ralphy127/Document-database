#include "Document.hpp"
#include "Logger.hpp"

#include <random>
#include <unordered_set>

class Collection {
public:
    Collection(std::string name) : _name(std::move(name)) {}

    void insert(Document doc);

    template<typename Filter, typename Modifier>
    std::vector<size_t> update(Filter&& filter, Modifier&& modify);

    void update(Document& doc);

    template<typename Filter>
    std::vector<Document> find(Filter&& filter);

    template<typename Filter>
    std::vector<size_t> remove(Filter&& filter);

    void remove(Document& doc);

    void insertVectorToDocument(std::vector<Document>& docs, std::string name, Document& doc);

    std::vector<Document> getAll() const { return _documents; }

    std::string getName() const { return _name; }

    std::optional<Document> getDocumentById(size_t id);

private:
    std::string _name;
    std::vector<Document> _documents;
    std::unordered_map<std::string, std::vector<size_t>> _indexes;
    std::unordered_set<size_t> _ids;

    std::mt19937_64 _rng{std::random_device{}()};



    size_t generateId();

    template<typename Filter>
    static constexpr void assert_filter() { static_assert(std::is_invocable_r_v<bool, Filter, const Document&>, 
        "Filter must be callable with const Document& and return bool"); }

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
            std::unordered_set<std::string> hadFields;
            for (const auto& [field, _] : _indexes) {
                if (doc.hasField(field)) {
                    hadFields.insert(field);
                }
            }

            modify(doc);

            for (auto& [field, positions] : _indexes) {
                auto had = static_cast<bool>(hadFields.count(field));
                auto hasNow = doc.hasField(field);

                auto it = std::find(positions.begin(), positions.end(), pos);

                if (had && !hasNow && it != positions.end()) {
                    positions.erase(it);
                } 
                else if (!had && hasNow && it == positions.end()) {
                    positions.push_back(pos);
                }
            }

            auto idOpt = doc.get<size_t>("id");
            if (idOpt) {
                idsUpdated.push_back(*idOpt);
                Logger::logInfo("Modified document of id: " + std::to_string(static_cast<size_t>(*idOpt)) + " in collection: " + _name + ".");
            } else {
                Logger::logWarning("Modified document with no id in collection: " + _name + '.');
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
        Logger::logWarning("Tried to remove non existing document in collection" + _name + '.');
    }

    for(auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
        size_t i{*it};
        auto id = _documents[i].get<size_t>("id").value_or(0);
        docIds.push_back(id);

        _ids.erase(id);

        for(auto& [field, positions] : _indexes) {
            positions.erase(std::remove(positions.begin(), positions.end(), i), positions.end());
            
            for(auto& pos : positions) {
                if(pos > i) {
                    --pos;
                }
            }
        }

        _documents.erase(_documents.begin() + i);
        Logger::logInfo("Removed document of id: " + std::to_string(id) + "in collection: " + _name + '.');
    }

    return docIds;
}