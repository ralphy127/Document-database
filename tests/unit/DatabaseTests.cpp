#include <gtest/gtest.h>

#include "Database.hpp"

class DatabaseTests : public ::testing::Test {
protected:
    std::string dbPath;
    Database db;
    std::string collectionName;

    DatabaseTests() : dbPath("testDatabase"), db(dbPath), collectionName("testCollection") {}

    void SetUp() override {
        db.addCollection(collectionName);
    }

    void TearDown() override {
        std::filesystem::remove_all(dbPath);
    }

    Document createDocumentWithId(size_t id, const std::string& name = "Doc") {
        Document doc;
        doc.set("id", id);
        doc.set("name", name);
        return doc;
    }
};

// -------------------- Tests: getCollection --------------------

TEST_F(DatabaseTests, GetCollection_WhenCollectionExists_ReturnReferenceToIt) {
    auto optCol = db.getCollection(collectionName);
    EXPECT_TRUE(optCol.has_value());
    Collection& col = optCol->get();
    EXPECT_EQ(col.getName(), collectionName);
}

TEST_F(DatabaseTests, GetCollection_WhenCollectionDoesNotExist_ReturnNullopt) {
    auto optCol = db.getCollection("nonexistent");
    ASSERT_FALSE(optCol.has_value());
}

// -------------------- Tests: addCollection --------------------

TEST_F(DatabaseTests, AddCollection_CreatesNewCollection) {
    std::string newCollection = "newCollection";
    db.addCollection(newCollection);
    auto optCol = db.getCollection(newCollection);

    ASSERT_TRUE(optCol.has_value());
    EXPECT_EQ(optCol->get().getName(), newCollection);
    EXPECT_TRUE(optCol->get().getAll().empty());
}

TEST_F(DatabaseTests, AddCollection_WhenNameIsDuplicated_DoesNotOverwrite) {
    auto doc = createDocumentWithId(1, "OriginalDoc");
    db.insert(collectionName, doc);

    db.addCollection(collectionName);
    auto optCol = db.getCollection(collectionName);
    ASSERT_TRUE(optCol.has_value());

    auto results = optCol->get().getAll();
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(*results[0].get<std::string>("name"), "OriginalDoc");
}

// -------------------- Tests: insertCollection --------------------

TEST_F(DatabaseTests, InsertCollection_AddsCollectionAndDocuments) {
    Collection col("insertedCol");
    Document doc;
    doc.set("name", std::string("doc_in_col"));
    col.insert(doc);
    db.insertCollection(col);
    auto optCol = db.getCollection("insertedCol");

    ASSERT_TRUE(optCol.has_value());
    auto docs = optCol->get().getAll();
    ASSERT_EQ(docs.size(), 1u);
    EXPECT_EQ(docs[0].get<std::string>("name"), std::optional<std::string>("doc_in_col"));
}

TEST_F(DatabaseTests, InsertCollection_WhenCollectionIsEmpty_Works) {
    Collection col("emptyCol");
    db.insertCollection(col);
    auto optCol = db.getCollection("emptyCol");
    ASSERT_TRUE(optCol.has_value());
    EXPECT_TRUE(optCol->get().getAll().empty());
}

TEST_F(DatabaseTests, InsertCollection_WhenNameIsDuplicated_DoesNotOverwrite) {
    Collection col1("dupCol");
    Document doc1; 
    doc1.set("name", std::string("first"));
    col1.insert(doc1);
    db.insertCollection(col1);

    Collection col2("dupCol");
    Document doc2; 
    doc2.set("name", std::string("second"));
    col2.insert(doc2);
    db.insertCollection(col2);

    auto optCol = db.getCollection("dupCol");
    ASSERT_TRUE(optCol.has_value());
    auto docs = optCol->get().getAll();
    ASSERT_EQ(docs.size(), 1u);
    EXPECT_EQ(docs[0].get<std::string>("name"), std::optional<std::string>("first"));
}

// -------------------- Tests: insert --------------------

TEST_F(DatabaseTests, Insert_AddsDocumentToCollection) {
    Document doc;
    doc.set("name", std::string("inserted_doc"));
    db.insert(collectionName, doc);
    auto optCol = db.getCollection(collectionName);
    ASSERT_TRUE(optCol.has_value());
    auto docs = optCol->get().getAll();
    auto found = std::find_if(docs.begin(), docs.end(), [](const Document& d) {
        return d.get<std::string>("name") == std::optional<std::string>("inserted_doc");
    });
    
    EXPECT_NE(found, docs.end());
    EXPECT_TRUE(found->get<size_t>("id").has_value());
}

TEST_F(DatabaseTests, Insert_WhenCollectionDoesNotExist_DoesNothing) {
    Document doc;
    doc.set("name", std::string("should_not_insert"));
    db.insert("nonexistent", doc);
    auto optCol = db.getCollection("nonexistent");
    EXPECT_FALSE(optCol.has_value());
}

TEST_F(DatabaseTests, Insert_WhenAddingMultipleDocuments_AllPresentAndUniqueIds) {
    Document doc1, doc2;
    doc1.set("name", std::string("doc1"));
    doc2.set("name", std::string("doc2"));
    db.insert(collectionName, doc1);
    db.insert(collectionName, doc2);
    auto optCol = db.getCollection(collectionName);
    ASSERT_TRUE(optCol.has_value());
    auto docs = optCol->get().getAll();
    std::unordered_set<size_t> ids;
    for (const auto& d : docs) {
        auto idOpt = d.get<size_t>("id");
        ASSERT_TRUE(idOpt.has_value());
        ids.insert(*idOpt);
    }
    EXPECT_EQ(ids.size(), docs.size());
}

TEST_F(DatabaseTests, Insert_WhenAddingDocumentWithExistingId_DoesNotDuplicate) {
    Document doc;
    doc.set("name", std::string("unique_doc"));
    db.insert(collectionName, doc);
    auto optCol = db.getCollection(collectionName);
    ASSERT_TRUE(optCol.has_value());
    auto docs = optCol->get().getAll();
    ASSERT_FALSE(docs.empty());
    auto idOpt = docs[0].get<size_t>("id");
    ASSERT_TRUE(idOpt.has_value());
    // Insert again with same id
    Document doc2;
    doc2.set("id", *idOpt);
    doc2.set("name", std::string("unique_doc"));
    db.insert(collectionName, doc2);
    auto docsAfter = optCol->get().getAll();
    size_t count = 0;
    for (const auto& d : docsAfter) {
        if (d.get<size_t>("id") == idOpt) count++;
    }
    EXPECT_EQ(count, 1u);
}

// -------------------- Tests: update<Filter, Modifier> --------------------

TEST_F(DatabaseTests, Update_UpdatesMatchingDocuments) {
    // Insert some docs
    Document doc1, doc2, doc3;
    doc1.set("name", std::string("doc1"));
    doc1.set("number", 1);
    doc2.set("name", std::string("doc2"));
    doc2.set("number", 2);
    doc3.set("name", std::string("doc3"));
    doc3.set("number", 3);
    db.insert(collectionName, doc1);
    db.insert(collectionName, doc2);
    db.insert(collectionName, doc3);

    // Update docs with number > 1
    db.update(collectionName,
        [](const Document& d) {
            auto num = d.get<int>("number");
            return num && *num > 1;
        },
        [](Document& d) {
            d.set("name", std::string("updated"));
        }
    );

    auto optCol = db.getCollection(collectionName);
    ASSERT_TRUE(optCol.has_value());
    auto docs = optCol->get().getAll();
    int updatedCount = 0;
    for (const auto& d : docs) {
        if (d.get<std::string>("name") == std::optional<std::string>("updated")) {
            ++updatedCount;
        }
    }
    EXPECT_EQ(updatedCount, 2);
}

TEST_F(DatabaseTests, Update_WhenCollectionDoesNotExist_DoesNothing) {
    EXPECT_NO_THROW(db.update("nonexistent",
        [](const Document&) { return true; },
        [](Document& d) { d.set("name", std::string("should_not_update")); }
    ));
}

TEST_F(DatabaseTests, Update_OnlyMatchingDocumentsAreUpdated) {
    Document doc1, doc2;
    doc1.set("name", std::string("doc1"));
    doc1.set("number", 1);
    doc2.set("name", std::string("doc2"));
    doc2.set("number", 2);
    db.insert(collectionName, doc1);
    db.insert(collectionName, doc2);

    db.update(collectionName,
        [](const Document& d) { return d.get<int>("number") == std::optional<int>(2); },
        [](Document& d) { d.set("name", std::string("only_this_updated")); }
    );

    auto optCol = db.getCollection(collectionName);
    ASSERT_TRUE(optCol.has_value());
    auto docs = optCol->get().getAll();
    int updatedCount = 0;
    for (const auto& d : docs) {
        if (d.get<std::string>("name") == std::optional<std::string>("only_this_updated")) {
            ++updatedCount;
        }
    }
    EXPECT_EQ(updatedCount, 1);
}

// -------------------- Tests: find<Filter> --------------------

TEST_F(DatabaseTests, Find_WhenFilteredByFieldValue_ReturnCorrectDocuments) {
    Document doc1, doc2, doc3;
    doc1.set("name", std::string("doc1")); doc1.set("number", 1);
    doc2.set("name", std::string("doc2")); doc2.set("number", 2);
    doc3.set("name", std::string("doc3")); doc3.set("number", 3);
    db.insert(collectionName, doc1);
    db.insert(collectionName, doc2);
    db.insert(collectionName, doc3);
    auto results = db.find(collectionName, [](const Document& d) {
        auto num = d.get<int>("number");
        return num && *num == 2;
    });
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].get<std::string>("name"), std::optional<std::string>("doc2"));
}

TEST_F(DatabaseTests, Find_WhenFilteredAll_ReturnAllDocuments) {
    Document doc1, doc2;
    doc1.set("name", std::string("doc1"));
    doc2.set("name", std::string("doc2"));
    db.insert(collectionName, doc1);
    db.insert(collectionName, doc2);
    auto results = db.find(collectionName, [](const Document&) { return true; });
    EXPECT_EQ(results.size(), 2u);
}

TEST_F(DatabaseTests, Find_WhenFilteredNone_ReturnEmpty) {
    Document doc1;
    doc1.set("name", std::string("doc1"));
    db.insert(collectionName, doc1);
    auto results = db.find(collectionName, [](const Document& d) {
        auto num = d.get<int>("number");
        return num && *num == 999;
    });
    EXPECT_TRUE(results.empty());
}

TEST_F(DatabaseTests, Find_WhenCollectionDoesNotExist_ReturnEmpty) {
    auto results = db.find("nonexistent", [](const Document&) { return true; });
    EXPECT_TRUE(results.empty());
}

// -------------------- Tests: remove<Filter> --------------------

TEST_F(DatabaseTests, Remove_WhenFilteredByFieldValue_RemoveCorrectDocuments) {
    Document doc1, doc2, doc3;
    doc1.set("name", std::string("doc1")); doc1.set("number", 1);
    doc2.set("name", std::string("doc2")); doc2.set("number", 2);
    doc3.set("name", std::string("doc3")); doc3.set("number", 3);
    db.insert(collectionName, doc1);
    db.insert(collectionName, doc2);
    db.insert(collectionName, doc3);
    db.remove(collectionName, [](const Document& d) {
        auto num = d.get<int>("number");
        return num && *num == 2;
    });
    auto results = db.find(collectionName, [](const Document&) { return true; });
    ASSERT_EQ(results.size(), 2u);
    for (const auto& d : results) {
        EXPECT_NE(d.get<int>("number"), std::optional<int>(2));
    }
}

TEST_F(DatabaseTests, Remove_WhenFilteredAll_RemoveAllDocuments) {
    Document doc1, doc2;
    doc1.set("name", std::string("doc1"));
    doc2.set("name", std::string("doc2"));
    db.insert(collectionName, doc1);
    db.insert(collectionName, doc2);
    db.remove(collectionName, [](const Document&) { return true; });
    auto results = db.find(collectionName, [](const Document&) { return true; });
    EXPECT_TRUE(results.empty());
}

TEST_F(DatabaseTests, Remove_WhenFilteredNone_DoNothing) {
    Document doc1;
    doc1.set("name", std::string("doc1"));
    db.insert(collectionName, doc1);
    db.remove(collectionName, [](const Document& d) {
        auto num = d.get<int>("number");
        return num && *num == 999;
    });
    auto results = db.find(collectionName, [](const Document&) { return true; });
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(DatabaseTests, Remove_WhenCollectionDoesNotExist_DoNothing) {
    EXPECT_NO_THROW(db.remove("nonexistent", [](const Document&) { return true; }));
}

// -------------------- Tests: remove --------------------

TEST_F(DatabaseTests, RemoveDocument_RemovesFromCollection) {
    Document doc;
    doc.set("name", std::string("to_remove"));
    db.insert(collectionName, doc);
    auto optCol = db.getCollection(collectionName);
    ASSERT_TRUE(optCol.has_value());
    auto docs = optCol->get().getAll();
    ASSERT_FALSE(docs.empty());
    auto idOpt = docs[0].get<size_t>("id");
    ASSERT_TRUE(idOpt.has_value());
    // Remove
    db.remove(collectionName, docs[0]);
    auto docsAfter = optCol->get().getAll();
    for (const auto& d : docsAfter) {
        EXPECT_NE(d.get<size_t>("id"), idOpt);
    }
}

TEST_F(DatabaseTests, RemoveDocument_WhenCollectionDoesntContainDocument_DoNothing) {
    Document doc;
    doc.set("id", static_cast<size_t>(9999999999));
    doc.set("name", std::string("should_not_remove"));
    auto docsBefore = db.getCollection(collectionName)->get().getAll();
    db.remove(collectionName, doc);
    auto docsAfter = db.getCollection(collectionName)->get().getAll();
    EXPECT_EQ(docsBefore, docsAfter);
}

TEST_F(DatabaseTests, RemoveDocument_WhenDocumentHasNoId_DoNothing) {
    Document doc;
    doc.set("name", std::string("should_not_remove"));
    auto docsBefore = db.getCollection(collectionName)->get().getAll();
    db.remove(collectionName, doc);
    auto docsAfter = db.getCollection(collectionName)->get().getAll();
    EXPECT_EQ(docsBefore, docsAfter);
}

TEST_F(DatabaseTests, RemoveDocument_WhenCollectionDoesNotExist_DoNothing) {
    Document doc;
    doc.set("name", std::string("should_not_remove"));
    EXPECT_NO_THROW(db.remove("nonexistent", doc));
}

// -------------------- Tests: insertContainerToDocument<Container< --------------------

TEST_F(DatabaseTests, InsertContainerToDocument_WhenDocIsNew_InsertsItWithContainer) {
    Document doc = createDocumentWithId(0);
    Document::Vector container;

    db.insertContainerToDocument(collectionName, container, "children", doc);

    auto optCol = db.getCollection(collectionName);
    ASSERT_TRUE(optCol.has_value());
    auto docs = optCol->get().getAll();
    ASSERT_EQ(docs.size(), 1);
    EXPECT_TRUE(docs[0].hasField("children"));
    EXPECT_TRUE(docs[0].get<Document::Vector>("children").has_value());
}

TEST_F(DatabaseTests, InsertContainerToDocument_WhenDocExists_UpdatesIt) {
    Document doc = createDocumentWithId(123);
    doc.set("name", std::string("parent"));
    db.insert(collectionName, doc);

    Document doc1, doc2, doc3;
    doc1.set<size_t>("id", 1u);
    doc2.set<size_t>("id", 2u);
    doc3.set<size_t>("id", 3u);
    Document::Vector container = {doc1, doc2, doc3};
    db.insertContainerToDocument(collectionName ,container, "children", doc);

    auto optCol = db.getCollection(collectionName);
    ASSERT_TRUE(optCol.has_value());
    auto found = optCol->get().getDocumentById(123);
    ASSERT_TRUE(found.has_value());

    auto vecOpt = found->get<Document::Vector>("children");
    ASSERT_TRUE(vecOpt.has_value());
    EXPECT_EQ(*vecOpt, container);
}

TEST_F(DatabaseTests, InsertContainerToDocument_FillsContainerWithIds) {

    Document doc = createDocumentWithId(0);
    db.insert(collectionName, doc);

    Document::Vector container;
    Document child1, child2;
    container.push_back(child1);
    container.push_back(child2);

    db.insertContainerToDocument(collectionName ,container, "children", doc);

    auto collection = db.getCollection(collectionName);
    ASSERT_TRUE(collection.has_value());
    auto documents = collection->get().getAll();
    ASSERT_EQ(documents.size(), 1u);
    auto vector = documents[0].get<Document::Vector>("children");
    ASSERT_TRUE(vector.has_value());
    EXPECT_TRUE((*vector)[0].get<size_t>("id").has_value());
    EXPECT_TRUE((*vector)[1].get<size_t>("id").has_value());
}

// -------------------- Tests: getAll --------------------

TEST_F(DatabaseTests, GetAll_ReturnsAllDocuments) {
    Document doc1, doc2;
    doc1.set("name", std::string("doc1"));
    doc2.set("name", std::string("doc2"));
    db.insert(collectionName, doc1);
    db.insert(collectionName, doc2);
    auto docs = db.getAll(collectionName);
    EXPECT_EQ(docs.size(), 2u);
    std::unordered_set<std::string> names;
    for (const auto& d : docs) {
        auto name = d.get<std::string>("name");
        ASSERT_TRUE(name.has_value());
        names.insert(*name);
    }
    EXPECT_EQ(names, std::unordered_set<std::string>({"doc1", "doc2"}));
}

TEST_F(DatabaseTests, GetAll_WhenCollectionIsEmpty_ReturnEmptyVector) {
    auto docs = db.getAll(collectionName);
    EXPECT_TRUE(docs.empty());
}

TEST_F(DatabaseTests, GetAll_WhenCollectionDoesNotExist_ReturnEmptyVector) {
    auto docs = db.getAll("nonexistent");
    EXPECT_TRUE(docs.empty());
}

TEST_F(DatabaseTests, GetAll_ReturnsCopyNotReference) {
    Document doc;
    doc.set("name", std::string("original"));
    db.insert(collectionName, doc);
    auto docs = db.getAll(collectionName);
    ASSERT_FALSE(docs.empty());
    docs[0].set("name", std::string("changed"));
    auto docsAfter = db.getAll(collectionName);
    EXPECT_NE(docsAfter[0].get<std::string>("name"), std::optional<std::string>("changed"));
}

// -------------------- Tests: getAll --------------------

 TEST_F(DatabaseTests, GetAll_ReturnsAllInsertedDocuments) {
    db.insert(collectionName, createDocumentWithId(1, "Doc1"));
    db.insert(collectionName, createDocumentWithId(2, "Doc2"));

    auto docs = db.getAll(collectionName);

    ASSERT_EQ(docs.size(), 2u);
    EXPECT_EQ(docs[0].get<size_t>("id"), 1u);
    EXPECT_EQ(docs[1].get<size_t>("id"), 2u);
}

TEST_F(DatabaseTests, GetAll_WhenCollectionDoesNotExist_ReturnsEmpty) {
    auto docs = db.getAll("nonexistent");
    EXPECT_TRUE(docs.empty());
}

TEST_F(DatabaseTests, GetAll_ReturnsCopy_NotReference) {
    db.insert(collectionName, createDocumentWithId(1));

    auto docs = db.getAll(collectionName);
    docs[0].set<size_t>("id", 999);

    auto original = db.getAll(collectionName);
    EXPECT_NE(original[0].get<size_t>("id"), 999);
}

// -------------------- Tests: getCollectionCopy --------------------

TEST_F(DatabaseTests, GetCollectionCopy_WhenCollectionExists_ReturnsIndependentCopy) {
    db.insert(collectionName, createDocumentWithId(1));

    auto colCopyOpt = db.getCollectionCopy(collectionName);
    ASSERT_TRUE(colCopyOpt.has_value());

    Collection& colCopy = colCopyOpt.value();
    auto docs = colCopy.getAll();
    ASSERT_EQ(docs.size(), 1u);
    EXPECT_EQ(docs[0].get<size_t>("id"), 1u);
}

TEST_F(DatabaseTests, GetCollectionCopy_ModifyingCopyDoesNotAffectOriginal) {
    db.insert(collectionName, createDocumentWithId(2));

    auto copyOpt = db.getCollectionCopy(collectionName);
    ASSERT_TRUE(copyOpt.has_value());

    Document extraDoc;
    extraDoc.set<size_t>("id", 999);
    copyOpt->insert(extraDoc);

    auto original = db.getCollection(collectionName);
    ASSERT_TRUE(original.has_value());
    EXPECT_EQ(original->get().getAll().size(), 1u);
}

TEST_F(DatabaseTests, GetCollectionCopy_WhenCollectionDoesNotExist_ReturnsNullopt) {
    auto copyOpt = db.getCollectionCopy("nonexistent");
    EXPECT_FALSE(copyOpt.has_value());
}