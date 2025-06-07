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
            collection.insert(doc);
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

    database.insertContainerToDocument("test_collection_1", vec, "nestedDocs", docs[0]);

    int mappedDocs{3};
    Document::Map map;
    for(int i{0}; i < mappedDocs; ++i) {
        Document doc;
        doc.set("name", std::string("mapped_") + std::to_string(i));
        doc.set("number", i);
        map.insert(std::make_pair("key_" + std::to_string(i), doc));
    }

    database.insertContainerToDocument("test_collection_2", map, "mappedDocs", docs[0]);
}

/*
---------------------------------------------------------------------------------------
Example of doc (test_database/test_collection_2/test_database/12526167272562074835.txt)
---------------------------------------------------------------------------------------
{
	nestedDocs (Document::Vector) : [
		[0]
		{
			number (int) : 0
			name (std::string) : nested_0
			id (size_t) : 83874144289562647
		}
		[1]
		{
			number (int) : 1
			name (std::string) : nested_1
			id (size_t) : 12917170294559764003
		}
		[2]
		{
			number (int) : 2
			name (std::string) : nested_2
			id (size_t) : 316956856605695197
		}
	]

	mappedDocs (Document::Map) : {
		key_0 : 
		{
			number (int) : 0
			name (std::string) : mapped_0
			id (size_t) : 16138790638605664726
		}
		key_2 : 
		{
			number (int) : 2
			name (std::string) : mapped_2
			id (size_t) : 17978313882910609125
		}
		key_1 : 
		{
			number (int) : 1
			name (std::string) : mapped_1
			id (size_t) : 10656443634445658548
		}
	}

	name (std::string) : document_1
	number (int) : 1
	id (size_t) : 12526167272562074835
}

---------------------------------------------------------------------------------------
Console output from main
---------------------------------------------------------------------------------------

[INFO] Added document of id: 13035354441807832731 in collection: test_collection_0.
[INFO] Added document of id: 8482643569069208438 in collection: test_collection_0.
[INFO] Added document of id: 18323238215660967790 in collection: test_collection_0.
[INFO] Added document of id: 11013485827546296107 in collection: test_collection_1.
[INFO] Added document of id: 12526167272562074835 in collection: test_collection_1.
[INFO] Added document of id: 17185698145303993609 in collection: test_collection_1.
[INFO] Added document of id: 12586214329053200915 in collection: test_collection_2.
[INFO] Added document of id: 5175530754003186045 in collection: test_collection_2.
[INFO] Added document of id: 6070036489475742309 in collection: test_collection_2.

Found 1 document(s) with name: document_1.

[INFO] Modified document of id: 11013485827546296107 in collection: test_collection_1.
[INFO] Modified document of id: 12526167272562074835 in collection: test_collection_1.
[INFO] Removed document of id: 18323238215660967790in collection: test_collection_0.
[INFO] Deleted document file: ../test_database/test_collection_0/18323238215660967790.txt.
[INFO] Updated document of id: 12526167272562074835 in collection: test_collection_1.
[INFO] Added document of id: 1400924974922546888 in collection: test_collection_2.
*/