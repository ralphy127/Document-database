#include "Database.hpp"

int main() {
    Database database("../test_database");

    int collections{3};
    for(int i{0}; i < collections; ++i) {
        std::string collectionName = "test_collection_" + std::to_string(i);
        Collection collection(collectionName);
        
        int docs{3};
        for(int i{0}; i < docs; ++i) {
            Document doc;
            doc.set("name", "document_" + std::to_string(i));
            doc.set("number", i);
            collection.insert(std::move(doc));
        }
    
        database.insertCollection(std::move(collection));
    }

    auto docs = database.find("test_collection_1", 
        [](const Document& doc) {
            auto name = doc.get<std::string>("name");
            return name && *name == "document_1";
        });

    std::cout << "\nFound " << docs.size() << " document(s) with name: document_1.\n" << std::endl;

    database.update( "test_collection_1",
        [](const Document& doc) {
            auto name = doc.get<std::string>("name");
            auto number = doc.get<int>("number");
            return (name && *name == "document_1") ||
                   (number && *number == 0);
        },
        [](Document& doc) {
            doc.set("name", std::string("document_111"));
            doc.set("number", 111);
        }
    );

    std::cout << "\nDatabase documents info: " << std::endl;
    for(const auto& doc : database.getAll("test_collection_1")) {
        doc.printInfo();
        std::cout << std::endl;
    }

    database.remove( "test_collection_0",
        [](const Document& doc) {
            auto name = doc.get<std::string>("name");
            return name && *name == "document_2";
        }
    );

    int nestedDocs{3};
    std::vector<Document> vec;
    for(int i{0}; i < nestedDocs; ++i) {
        Document doc;
        doc.set("name", std::string("nested_") + std::to_string(i));
        doc.set("number", i);
        vec.push_back(doc);
    }

    database.insertVectorToDocument("test_collection_1", vec, "nestedDocs", docs[0]);
}

/*
********** main() output **********
[INFO] Added document of id: 9593768778106862104 in collection: test_collection_0.
[INFO] Added document of id: 16842304801838006857 in collection: test_collection_0.
[INFO] Added document of id: 10943324034815757192 in collection: test_collection_0.
[INFO] Added document of id: 1189554868122494931 in collection: test_collection_1.
[INFO] Added document of id: 11876987865039192103 in collection: test_collection_1.
[INFO] Added document of id: 355868193621290455 in collection: test_collection_1.
[INFO] Added document of id: 13874720406315364869 in collection: test_collection_2.
[INFO] Added document of id: 7744287572528817370 in collection: test_collection_2.
[INFO] Added document of id: 13816884646374761983 in collection: test_collection_2.

Found 1 document(s) with name: document_1.

[INFO] Modified document of id: 1189554868122494931 in collection: test_collection_1.
[INFO] Modified document of id: 11876987865039192103 in collection: test_collection_1.

Database documents info: 
name: document_111
number: 111
id: 1189554868122494931

name: document_111
number: 111
id: 11876987865039192103

name: document_2
number: 2
id: 355868193621290455

[INFO] Removed document of id: 10943324034815757192in collection: test_collection_0.
[INFO] Deleted document file: ../test_database/test_collection_0/10943324034815757192.txt
[INFO] Added document of id: 5114969041708274658 in collection: test_collection_1.
*/