#include <gtest/gtest.h>

#include "Collection.hpp"

class CollectionTest : public ::testing::Test {
protected:
    Collection collection = Collection("TestCollection");

    void SetUp() override {
        Document doc1;
        doc1.set("name", std::string("test_1"));
        doc1.set("number", 1);

        Document doc2;
        doc2.set("name", std::string("test_2"));
        doc2.set("number", 2);

        Document doc3;
        doc3.set("name", std::string("test_3"));
        doc3.set("number", 3);

        collection.insert(doc1);
        collection.insert(doc2);
        collection.insert(doc3);
    }
};

// -------------------- Tests: insert --------------------

TEST_F(CollectionTest, Insert_IncreasesCollectionSize) {
    Collection col("InsertTest");
    Document doc;

    EXPECT_EQ(col.getAll().size(), 0u);
    col.insert(doc);
    EXPECT_EQ(col.getAll().size(), 1u);
}

TEST_F(CollectionTest, Insert_AssignsUniqueId) {
    Collection col("InsertTest");
    Document doc;

    col.insert(doc);
    auto docs = col.getAll();
    ASSERT_EQ(docs.size(), 1u);
    auto idOpt = docs[0].get<size_t>("id");
    EXPECT_TRUE(idOpt.has_value());
}

TEST_F(CollectionTest, Insert_InsertedDocumentFieldsAreCorrect) {
    Collection col("InsertTest");
    Document doc;
    doc.set("name", std::string("inserted_doc"));
    doc.set("number", 42);

    col.insert(doc);
    auto docs = col.getAll();
    ASSERT_EQ(docs.size(), 1u);
    EXPECT_EQ(docs[0].get<std::string>("name"), std::optional<std::string>("inserted_doc"));
    EXPECT_EQ(docs[0].get<int>("number"), std::optional<int>(42));
}

TEST_F(CollectionTest, Insert_MultipleDocumentsHaveUniqueIds) {
    Collection col("InsertTest");
    Document doc1, doc2;
    doc1.set("name", std::string("doc1"));
    doc2.set("name", std::string("doc2"));
    col.insert(doc1);
    col.insert(doc2);
    
    auto docs = col.getAll();
    ASSERT_EQ(docs.size(), 2u);
    auto id1 = docs[0].get<size_t>("id");
    auto id2 = docs[1].get<size_t>("id");
    ASSERT_TRUE(id1.has_value() && id2.has_value());
    EXPECT_NE(*id1, *id2);
}

TEST_F(CollectionTest, Insert_WhenInsertingNestedVector_AreAssignedIds) {
    Document nested1, nested2;
    nested1.set("field",std::string( "value1"));
    nested2.set("field", std::string("value2"));

    Document::Vector nestedVector = { nested1, nested2 };

    Document doc;
    doc.set("name", std::string("parent_doc"));
    doc.set("children", nestedVector);

    collection.insert(doc);

    auto results = collection.find([](const Document& d) {
        return d.get<std::string>("name") == std::optional<std::string>("parent_doc");
    });
    ASSERT_EQ(results.size(), 1u);

    auto childrenOpt = results[0].get<Document::Vector>("children");
    ASSERT_TRUE(childrenOpt.has_value());
    ASSERT_EQ(childrenOpt->size(), 2u);

    for (const auto& child : *childrenOpt) {
        auto idOpt = child.get<size_t>("id");
        EXPECT_TRUE(idOpt.has_value()) << "Nested document does not have an id.";
    }
}

TEST_F(CollectionTest, Insert_WhenInsertingNestedMap_AreAssignedIds) {
    Document nested1, nested2;
    nested1.set("field", std::string("value1"));
    nested2.set("field", std::string("value2"));

    Document::Map nestedMap;
    nestedMap["first"] = nested1;
    nestedMap["second"] = nested2;

    Document doc;
    doc.set("name", std::string("parent_doc_map"));
    doc.set("children", nestedMap);

    collection.insert(doc);

    auto results = collection.find([](const Document& d) {
        return d.get<std::string>("name") == std::optional<std::string>("parent_doc_map");
    });
    ASSERT_EQ(results.size(), 1u);

    auto childrenOpt = results[0].get<Document::Map>("children");
    ASSERT_TRUE(childrenOpt.has_value());
    ASSERT_EQ(childrenOpt->size(), 2u);

    for (const auto& [key, child] : *childrenOpt) {
        auto idOpt = child.get<size_t>("id");
        EXPECT_TRUE(idOpt.has_value()) << "Nested document in map does not have an id.";
    }
}


// -------------------- Tests: update<Filter, Modifier> --------------------

TEST_F(CollectionTest, Update_FilterModifier_UpdatesMatchingDocuments) {
    // Update all documents with number > 1, set name to "updated"
    auto updatedIds = collection.update(
        [](const Document& doc) {
            auto num = doc.get<int>("number");
            return num.has_value() && *num > 1;
        },
        [](Document& doc) {
            doc.set("name", std::string("updated"));
        }
    );

    // Should update doc2 and doc3
    EXPECT_EQ(updatedIds.size(), 2u);
    auto docs = collection.getAll();
    size_t updatedCount = 0;
    for (const auto& doc : docs) {
        auto name = doc.get<std::string>("name");
        if (name && *name == "updated") {
            ++updatedCount;
        }
    }

    EXPECT_EQ(updatedCount, 2);
}

TEST_F(CollectionTest, Update_FilterModifier_ReturnsCorrectIds) {
    // Get ids before update
    auto docs = collection.getAll();
    std::vector<size_t> expectedIds;
    for (const auto& doc : docs) {
        if (doc.get<int>("number") == std::optional<int>(1)) {
            auto id = doc.get<size_t>("id");
            if (id) expectedIds.push_back(*id);
        }
    }
    // Update doc with number == 1
    auto updatedIds = collection.update(
        [](const Document& doc) {
            return doc.get<int>("number") == std::optional<int>(1);
        },
        [](Document& doc) {
            doc.set("name", std::string("first_updated"));
        }
    );

    EXPECT_EQ(updatedIds, expectedIds);
}

TEST_F(CollectionTest, Update_FilterModifier_NoMatchDoesNothing) {
    // Try to update with a filter that matches nothing
    auto updatedIds = collection.update(
        [](const Document& doc) {
            return doc.get<int>("number") == std::optional<int>(999);
        },
        [](Document& doc) {
            doc.set("name", std::string("should_not_update"));
        }
    );

    EXPECT_TRUE(updatedIds.empty());
    // Ensure no document was changed
    auto docs = collection.getAll();
    for (const auto& doc : docs) {
        EXPECT_NE(doc.get<std::string>("name"), std::optional<std::string>("should_not_update"));
    }
}

// -------------------- Tests: update --------------------

TEST_F(CollectionTest, Update_WhenIdIsValid_UpdateDocument) {
    // Get a document from the collection
    auto docs = collection.getAll();
    ASSERT_FALSE(docs.empty());
    Document doc = docs[0];
    auto idOpt = doc.get<size_t>("id");
    ASSERT_TRUE(idOpt.has_value());
    // Change a field

    doc.set("name", std::string("updated_name"));
    // Update in collection
    collection.update(doc);

    // Check the change is reflected
    auto updatedDocOpt = collection.getDocumentById(*idOpt);
    ASSERT_TRUE(updatedDocOpt.has_value());
    EXPECT_EQ(updatedDocOpt->get<std::string>("name"), std::optional<std::string>("updated_name"));
}

TEST_F(CollectionTest, UpdateDocument_WhenIdDoesNotExist_DoesNothing) {
    // Create a document with a random id not in the collection
    Document doc;
    doc.set("id", static_cast<size_t>(9999999999));
    doc.set("name", std::string("should_not_update"));

    // Get state before
    auto docsBefore = collection.getAll();
    // Try to update
    collection.update(doc);

    // State should be unchanged
    auto docsAfter = collection.getAll();
    EXPECT_EQ(docsBefore, docsAfter);
}

TEST_F(CollectionTest, UpdateDocument_WhenDocHasNoId_DoesNothing) {
    // Create a document with no id
    Document doc;
    doc.set("name", std::string("should_not_update"));

    // Get state before
    auto docsBefore = collection.getAll();
    // Try to update
    collection.update(doc);

    // State should be unchanged
    auto docsAfter = collection.getAll();
    EXPECT_EQ(docsBefore, docsAfter);
}

// -------------------- Tests: find<Filter> --------------------

TEST_F(CollectionTest, Find_WhenFilteredByFieldValue_ReturnsCorrectDocuments) {
    // Find documents with number == 1 v number == 2
    auto results = collection.find([](const Document& doc) {
        auto number = doc.get<int>("number");
        return number && (*number == 1 || *number == 2);
    });

    EXPECT_EQ(results.size(), 2u);
    std::unordered_set<int> expectedNumbers = {1, 2};
    std::unordered_set<int> foundNumbers;

    for (const auto& doc : results) {
        auto number = doc.get<int>("number");
        ASSERT_TRUE(number.has_value());
        foundNumbers.insert(*number);
    }
    EXPECT_EQ(foundNumbers, expectedNumbers);
}

TEST_F(CollectionTest, Find_WhenFilteredAll_ReturnsAllDocuments) {
    auto results = collection.find([](const Document&) { return true; });
    EXPECT_EQ(results.size(), 3u);
}

TEST_F(CollectionTest, Find_WhenFilteredNone_ReturnsEmpty) {
    auto results = collection.find([](const Document& doc) {
        return doc.get<int>("number") == std::optional<int>(999);
    });
    EXPECT_TRUE(results.empty());
}

// -------------------- Tests: remove<Filter> --------------------

TEST_F(CollectionTest, Remove_ReturnsCorrectIds_WhenFilteringByField) {
    auto docsBefore = collection.getAll();
    std::vector<size_t> expectedIds;

    for (const auto& doc : docsBefore) {
        auto number = doc.get<int>("number");
        if (number && (*number == 1 || *number == 2)) {
            auto id = doc.get<size_t>("id");
            if (id) expectedIds.push_back(*id);
        }
    }

    auto removedIds = collection.remove([](const Document& doc) {
        auto number = doc.get<int>("number");
        return number && (*number == 1 || *number == 2);
    });

    std::sort(removedIds.begin(), removedIds.end());
    std::sort(expectedIds.begin(), expectedIds.end());

    EXPECT_EQ(removedIds, expectedIds);
}

TEST_F(CollectionTest, Remove_PhysicallyRemovesMatchingDocuments) {
    collection.remove([](const Document& doc) {
        auto number = doc.get<int>("number");
        return number && (*number == 1 || *number == 2);
    });

    auto docsAfter = collection.getAll();
    for (const auto& doc : docsAfter) {
        auto number = doc.get<int>("number");
        EXPECT_FALSE(number && (*number == 1 || *number == 2));
    }
}

TEST_F(CollectionTest, Remove_FilterAll_RemovesAllDocuments) {
    auto removedIds = collection.remove([](const Document&) { return true; });
    EXPECT_EQ(removedIds.size(), 3u);
    EXPECT_TRUE(collection.getAll().empty());
}

TEST_F(CollectionTest, Remove_FilterNone_DoesNothing) {
    auto docsBefore = collection.getAll();
    auto removedIds = collection.remove([](const Document& doc) {
        auto number = doc.get<int>("number");
        return number && *number == 999;
    });
    EXPECT_TRUE(removedIds.empty());
    auto docsAfter = collection.getAll();
    EXPECT_EQ(docsBefore, docsAfter);
}

// -------------------- Tests: remove(Document&) --------------------

TEST_F(CollectionTest, RemoveDocument_WhenIdIsValid_RemoveDocument) {
    // Get a document from the collection
    auto docs = collection.getAll();
    ASSERT_FALSE(docs.empty());
    Document doc = docs[0];
    auto idOpt = doc.get<size_t>("id");
    ASSERT_TRUE(idOpt.has_value());

    // Remove the document
    collection.remove(doc);

    // Ensure it is removed
    auto docsAfter = collection.getAll();
    for (const auto& d : docsAfter) {
        auto id = d.get<size_t>("id");
        ASSERT_TRUE(id.has_value());
        EXPECT_NE(*id, *idOpt);
    }
}

TEST_F(CollectionTest, RemoveDocument_WhenIdIsNotValid_DoNothing) {
    // Create a document with a random id not in the collection
    Document doc;
    doc.set("id", static_cast<size_t>(9999999999));
    doc.set("name", std::string("should_not_remove"));
    // Get state before
    auto docsBefore = collection.getAll();
    // Try to remove
    collection.remove(doc);
    // State should be unchanged
    auto docsAfter = collection.getAll();
    EXPECT_EQ(docsBefore, docsAfter);
}

TEST_F(CollectionTest, RemoveDocument_WhenThereIsNoId_DoNothing) {
    // Create a document with no id
    Document doc;
    doc.set("name", std::string("should_not_remove"));
    // Get state before
    auto docsBefore = collection.getAll();
    // Try to remove
    collection.remove(doc);
    // State should be unchanged
    auto docsAfter = collection.getAll();
    EXPECT_EQ(docsBefore, docsAfter);
}

// -------------------- Tests: insertContainerToDocument<Container> --------------------

TEST_F(CollectionTest, InsertContainerToDocument_WhenInsertingVector_AddsContainerAndAssignsIds) {
    Document::Vector vec;
    Document d1, d2;
    d1.set("field", 1);
    d2.set("field", 2);
    vec.push_back(d1);
    vec.push_back(d2);

    Document doc;
    doc.set("name", std::string("container_doc"));
    // Should create doc in collection if not present
    collection.insertContainerToDocument(vec, "my_vector", doc);

    // Find the document in the collection
    auto found = collection.find([](const Document& d) {
        return d.get<std::string>("name") == std::optional<std::string>("container_doc");
    });
    ASSERT_EQ(found.size(), 1u);

    // Check the container is present and ids are assigned
    auto containerOpt = found[0].get<Document::Vector>("my_vector");
    ASSERT_TRUE(containerOpt.has_value());
    for (const auto& subdoc : *containerOpt) {
        EXPECT_TRUE(subdoc.get<size_t>("id").has_value());
    }
}

TEST_F(CollectionTest, InsertContainerToDocument_WhenInstMap_AddsContainerAndAssignsIds) {
    Document::Map map;
    Document d1, d2;
    d1.set("field", 1);
    d2.set("field", 2);
    map["a"] = d1;
    map["b"] = d2;

    Document doc;
    doc.set("name", std::string("container_doc_map"));
    // Should create doc in collection if not present
    collection.insertContainerToDocument(map, "my_map", doc);

    // Find the document in the collection
    auto found = collection.find([](const Document& d) {
        return d.get<std::string>("name") == std::optional<std::string>("container_doc_map");
    });
    ASSERT_EQ(found.size(), 1u);
    // Check the container is present and ids are assigned
    auto containerOpt = found[0].get<Document::Map>("my_map");
    ASSERT_TRUE(containerOpt.has_value());
    for (const auto& [key, subdoc] : *containerOpt) {
        EXPECT_TRUE(subdoc.get<size_t>("id").has_value());
    }
}

TEST_F(CollectionTest, InsertContainerToDocument_WhenDocExists_UpdatesContainer) {
    // Insert a doc
    Document doc;
    doc.set("name", std::string("existing_doc"));
    collection.insert(doc);
    // Prepare a vector
    Document::Vector vec;
    Document d1; 
    vec.push_back(d1);
    // Insert container to the existing doc
    collection.insertContainerToDocument(vec, "vec_field", doc);
    
    // Find and check
    auto found = collection.find([](const Document& d) {
        return d.get<std::string>("name") == std::optional<std::string>("existing_doc");
    });
    ASSERT_EQ(found.size(), 1u);
    auto containerOpt = found[0].get<Document::Vector>("vec_field");
    ASSERT_TRUE(containerOpt.has_value());
    EXPECT_EQ(containerOpt->size(), 1u);
    EXPECT_TRUE((*containerOpt)[0].get<size_t>("id").has_value());
}

TEST_F(CollectionTest, InsertContainerToDocument_WhenContainerIsEmpty_StillCreatesField) {
    Document::Vector vec;
    Document doc;
    doc.set("name", std::string("empty_container_test"));
    collection.insertContainerToDocument(vec, "empty_vector", doc);

    auto result = collection.find([](const Document& d) {
        return d.get<std::string>("name") == std::optional<std::string>("empty_container_test");
    });
    ASSERT_EQ(result.size(), 1u);
    auto containerOpt = result[0].get<Document::Vector>("empty_vector");
    EXPECT_TRUE(containerOpt.has_value());
    EXPECT_TRUE(containerOpt->empty());
}

// -------------------- Tests: getAll --------------------

TEST_F(CollectionTest, GetAll_ReturnsAllDocuments) {
    // Initial state: 3 documents
    auto docs = collection.getAll();
    EXPECT_EQ(docs.size(), 3u);
    std::unordered_set<std::string> expectedNames = {"test_1", "test_2", "test_3"};
    std::unordered_set<std::string> foundNames;
    for (const auto& doc : docs) {
        auto name = doc.get<std::string>("name");
        ASSERT_TRUE(name.has_value());
        foundNames.insert(*name);
    }
    EXPECT_EQ(foundNames, expectedNames);
}

TEST_F(CollectionTest, GetAll_ReturnsCopy_NotReference) {
    // Modify returned vector, should not affect collection
    auto docs = collection.getAll();
    ASSERT_FALSE(docs.empty());
    docs[0].set("name", std::string("changed"));
    auto docsAfter = collection.getAll();
    EXPECT_NE(docsAfter[0].get<std::string>("name"), std::optional<std::string>("changed"));
}

TEST_F(CollectionTest, GetAll_ReflectsCurrentState) {
    // Remove one document, getAll should reflect
    auto docs = collection.getAll();
    ASSERT_FALSE(docs.empty());
    collection.remove(docs[0]);
    auto docsAfter = collection.getAll();
    EXPECT_EQ(docsAfter.size(), 2u);
}

// -------------------- Tests: fillContainerWithIds --------------------

TEST_F(CollectionTest, FillContainerWithIds_Vector_AssignsUniqueIds) {
    Document::Vector vec(3);
    vec[0].set("field", 1);
    vec[1].set("field", 2);
    vec[2].set("field", 3);
    collection.fillContainerWithIds(vec);
    std::unordered_set<size_t> ids;
    for (const auto& doc : vec) {
        auto idOpt = doc.get<size_t>("id");
        ASSERT_TRUE(idOpt.has_value());
        ids.insert(*idOpt);
    }
    EXPECT_EQ(ids.size(), 3u);
}

TEST_F(CollectionTest, FillContainerWithIds_Map_AssignsUniqueIds) {
    Document::Map map;
    map["a"].set("field", 1);
    map["b"].set("field", 2);
    map["c"].set("field", 3);
    collection.fillContainerWithIds(map);
    std::unordered_set<size_t> ids;
    for (const auto& [k, doc] : map) {
        auto idOpt = doc.get<size_t>("id");
        ASSERT_TRUE(idOpt.has_value());
        ids.insert(*idOpt);
    }
    EXPECT_EQ(ids.size(), 3u);
}

TEST_F(CollectionTest, FillContainerWithIds_DoesNotChangeExistingIds) {
    Document::Vector vec(2);
    vec[0].set("id", static_cast<size_t>(12345));
    collection.fillContainerWithIds(vec);
    EXPECT_EQ(vec[0].get<size_t>("id"), std::optional<size_t>(12345));
    EXPECT_TRUE(vec[1].get<size_t>("id").has_value());
}

TEST_F(CollectionTest, FillContainerWithIds_EmptyContainer_NoCrash) {
    Document::Vector vec;
    EXPECT_NO_THROW(collection.fillContainerWithIds(vec));
    EXPECT_TRUE(vec.empty());
    Document::Map map;
    EXPECT_NO_THROW(collection.fillContainerWithIds(map));
    EXPECT_TRUE(map.empty());
}

// -------------------- Tests: getDocumentById --------------------

TEST_F(CollectionTest, GetDocumentById_ReturnsCorrectDocument) {
    auto docs = collection.getAll();
    ASSERT_FALSE(docs.empty());
    for (const auto& doc : docs) {
        auto idOpt = doc.get<size_t>("id");
        ASSERT_TRUE(idOpt.has_value());
        auto found = collection.getDocumentById(*idOpt);
        ASSERT_TRUE(found.has_value());
        EXPECT_EQ(found->get<size_t>("id"), idOpt);
        EXPECT_EQ(found->get<std::string>("name"), doc.get<std::string>("name"));
    }
}

TEST_F(CollectionTest, GetDocumentById_NonExistentId_ReturnsNullopt) {
    auto result = collection.getDocumentById(999999);
    EXPECT_FALSE(result.has_value());
}