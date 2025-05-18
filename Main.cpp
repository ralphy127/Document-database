#include <Storage.hpp>
#include "Collection.hpp"

#include <iostream>

int main() {
    Collection collection("test_collection");

    Document doc1;
    doc1.set("name", std::string("Jan"));
    doc1.set("age", 30);
    collection.insert(std::move(doc1));

    Document doc2;
    doc2.set("name", std::string("Anna"));
    doc2.set("age", 25);
    collection.insert(std::move(doc2));

    Document doc3;
    doc3.set("name", std::string("Piotr"));
    doc3.set("age", 40);
    collection.insert(doc3);

    for(const auto& doc : collection.getAll()) {
        doc.printInfo();
        std::cout << std::endl;
    }

    auto docs = collection.find([](const Document& doc) {
        auto name = doc.get<std::string>("name");
        return name && *name == "Jan";
    });

    std::cout << "Found " << docs.size() << " document(s) with name Jan.\n" << std::endl;

    collection.update(
        [](const Document& doc) {
            auto name = doc.get<std::string>("name");
            return name && *name == "Piotr";
        },
        [](Document& doc) {
            doc.set("age", 41);
        }
    );

    for(const auto& doc : collection.getAll()) {
        doc.printInfo();
        std::cout << std::endl;
    }

    collection.remove(
        [](const Document& doc) {
            auto name = doc.get<std::string>("name");
            return name && *name == "Anna";
        }
    );

    for(auto& doc : collection.getAll()) {
        Document nestedDoc;
        nestedDoc.set("name", std::string("Agnieszka"));
        nestedDoc.set("age", 33);
        std::vector<Document> vec{nestedDoc};
        collection.insertVectorToDocument(vec, "vector", doc); 
    }

    for(const auto& doc : collection.getAll()) {
        doc.printInfo();
        std::cout << std::endl;
    }

    

    Storage storage("../test_database");
    for(const auto& doc : collection.getAll()) {
        storage.saveDocument(doc);
    }
}