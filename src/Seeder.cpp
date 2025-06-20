#include "Seeder.hpp"

#include "Database.hpp"


void Seeder::seedDatabase(Database& db) {
    if(!db.empty()) {
        Logger::logInfo("Database " + db.getName() + " is not empty, skipped seeding.");
        return;
    }

    // Collection 1
    db.addCollection("example_collection");

    Document doc1;
    doc1.set("name", std::string("Document One"));

    Document::Map addressMap;

    Document streetDoc;
    streetDoc.set("value", std::string("Main Street"));
    addressMap["street"] = streetDoc;

    Document numberDoc;
    numberDoc.set("value", 42);
    addressMap["number"] = numberDoc;

    Document::Map locationMap;

    Document cityDoc;
    cityDoc.set("value", std::string("New York"));
    locationMap["city"] = cityDoc;

    Document zipDoc;
    zipDoc.set("value", std::string("10001"));
    locationMap["zip"] = zipDoc;

    Document locationDoc;
    locationDoc.set("value", locationMap);
    addressMap["location"] = locationDoc;

    doc1.set("address", addressMap);

    Document::Vector tags;
    Document tag1, tag2, tag3;
    tag1.set("value", 1);
    tag2.set("value", 2);
    tag3.set("value", 2);
    tags.push_back(tag1);
    tags.push_back(tag2);
    tags.push_back(tag3);
    doc1.set("tags", tags);

    Document doc2;
    doc2.set("name", std::string("Parent Document"));

    Document::Vector children;
    Document child1;
    child1.set("name", std::string("Child One"));
    child1.set("age", 10);

    Document child2;
    child2.set("name", std::string("Child Two"));
    child2.set("age", 12);

    children.push_back(child1);
    children.push_back(child2);

    doc2.set("children", children);

    Document doc3;
    doc3.set("name", std::string("Matrix Holder"));

    Document::Vector row1, row2;

    row1.push_back(Document());
    row1.push_back(Document());
    row2.push_back(Document());
    row2.push_back(Document());

    Document docr1, docr2;
    docr1.set("row1", row1);
    docr1.set("row2", row2);

    Document::Vector matrix;
    matrix.push_back(docr1);
    matrix.push_back(docr2);

    doc3.set("matrix", matrix);

    Document doc4;
    doc4.set("name", std::string("Complex One"));

    Document::Map metadata;

    Document versionDoc;
    versionDoc.set("value", 1.2);
    metadata["version"] = versionDoc;

    Document activeDoc;
    activeDoc.set("value", true);
    metadata["active"] = activeDoc;

    Document::Vector innerDocs;
    Document a, b;
    a.set("meta", std::string("A"));
    b.set("meta", std::string("B"));
    innerDocs.push_back(a);
    innerDocs.push_back(b);

    Document docsDoc;
    docsDoc.set("value", innerDocs);
    metadata["docs"] = docsDoc;

    doc4.set("metadata", metadata);

    db.insert("example_collection", doc1);
    db.insert("example_collection", doc2);
    db.insert("example_collection", doc3);
    db.insert("example_collection", doc4);

    // Collection 2
    db.addCollection("simple_collection");

    Document simple1;
    simple1.set("title", std::string("Simple Doc 1"));
    simple1.set("value", 123);
    db.insert("simple_collection", simple1);

    Document simple2;
    simple2.set("title", std::string("Simple Doc 2"));
    simple2.set("active", false);
    db.insert("simple_collection", simple2);

    // Collection 3 
    db.addCollection("mixed_collection");

    Document mixed1;
    mixed1.set("description", std::string("Mixed Doc with nested vector"));
    Document::Vector numbers;

    Document num1;
    num1.set("value", 10);
    numbers.push_back(num1);

    Document num2;
    num2.set("value", 20);
    numbers.push_back(num2);

    Document num3;
    num3.set("value", 30);
    numbers.push_back(num3);

    mixed1.set("numbers", numbers);

    db.insert("mixed_collection", mixed1);

    Document mixed2;
    Document::Map info;

    Document fooDoc;
    fooDoc.set("value", std::string("bar"));
    info["foo"] = fooDoc;

    Document bazDoc;
    bazDoc.set("value", 42);
    info["baz"] = bazDoc;

    mixed2.set("info", info);

    db.insert("mixed_collection", mixed2);


    std::cout << "Example database seeded with 3 collections.\n";
}
