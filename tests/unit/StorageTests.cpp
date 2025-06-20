#include <gtest/gtest.h>

#include "Storage.hpp"

#include <fstream>

class StorageTests : public ::testing::Test {
protected:
    Storage storage;
    std::string collectionPath = "test_output";

    void SetUp() override {
        if (!std::filesystem::exists(collectionPath)) {
            std::filesystem::create_directory(collectionPath);
        }
    }

    void TearDown() override {
        std::filesystem::remove_all(collectionPath);
    }

    Document createSampleDocument(size_t id) {
        Document doc;
        doc.set("id", id);
        doc.set("name", std::string("Test Document"));
        doc.set("value", 42);
        return doc;
    }
};

// -------------------- Tests: saveDocument --------------------

TEST_F(StorageTests, SaveDocument_CreatesFile) {
    auto doc = createSampleDocument(11);

    storage.saveDocument(collectionPath, doc);

    std::string expectedPath = collectionPath + "/11.txt";
    EXPECT_TRUE(std::filesystem::exists(expectedPath));
}

TEST_F(StorageTests, SaveDocument_WhenDocumentIsNested_CreatesExpectedFile) {
    Document inner;
    inner.set<size_t>("id", 111);
    inner.set("value", 123);

    Document doc;
    doc.set<size_t>("id", 12);
    doc.set("nested", inner);

    storage.saveDocument(collectionPath, doc);

    auto files = std::filesystem::directory_iterator(collectionPath);
    size_t fileCount = std::distance(begin(files), end(files));
    EXPECT_EQ(fileCount, 1);

    for (const auto& entry : std::filesystem::directory_iterator(collectionPath)) {
        std::ifstream file(entry.path());
        std::string line;
        bool foundValue = false;
        while (std::getline(file, line)) {
            if (line.find("value") != std::string::npos && line.find("123") != std::string::npos) {
                foundValue = true;
                break;
            }
        }
        EXPECT_TRUE(foundValue);
    }
}

TEST_F(StorageTests, SaveDocument_WhenVectorIsNested_CreatesExpectedFile) {
    Document vectorElem;
    vectorElem.set("val", 789);

    Document::Vector vec;
    vec.push_back(vectorElem);
    vec.push_back(vectorElem);

    Document doc;
    doc.set<size_t>("id", 13);
    doc.set("nestedVector", vec);

    storage.saveDocument(collectionPath, doc);

    auto files = std::filesystem::directory_iterator(collectionPath);
    size_t fileCount = std::distance(begin(files), end(files));
    EXPECT_EQ(fileCount, 1);

    for (const auto& entry : std::filesystem::directory_iterator(collectionPath)) {
        std::ifstream file(entry.path());
        std::string line;
        bool foundVal = false;
        size_t valCount = 0;

        while (std::getline(file, line)) {
            if (line.find("val") != std::string::npos && line.find("789") != std::string::npos) {
                foundVal = true;
                valCount++;
            }
        }

        EXPECT_TRUE(foundVal);
        EXPECT_EQ(valCount, 2);
    }
}

TEST_F(StorageTests, SaveDocument_WhenMapIsNested_CreatesExpectedFile) {
    Document mapElem;
    mapElem.set("val", 456);

    Document::Map map;
    map["first"] = mapElem;
    map["second"] = mapElem;

    Document doc;
    doc.set<size_t>("id", 14);
    doc.set("nestedMap", map);

    storage.saveDocument(collectionPath, doc);

    auto files = std::filesystem::directory_iterator(collectionPath);
    size_t fileCount = std::distance(begin(files), end(files));
    EXPECT_EQ(fileCount, 1);

    for (const auto& entry : std::filesystem::directory_iterator(collectionPath)) {
        std::ifstream file(entry.path());
        std::string line;
        bool foundVal = false;
        size_t valCount = 0;

        while (std::getline(file, line)) {
            if (line.find("val") != std::string::npos && line.find("456") != std::string::npos) {
                foundVal = true;
                valCount++;
            }
        }

        EXPECT_TRUE(foundVal);
        EXPECT_EQ(valCount, 2);
    }
}

// -------------------- Tests: loadDocuments --------------------

TEST_F(StorageTests, LoadDocuments_ReadsBackData) {
    auto doc = createSampleDocument(21);

    storage.saveDocument(collectionPath, doc);

    auto loaded = storage.loadDocuments(collectionPath);
    EXPECT_EQ(loaded.size(), 1);
    EXPECT_EQ(*loaded[0].get<std::string>("name"), "Test Document");
    EXPECT_EQ(*loaded[0].get<int>("value"), 42);
}

TEST_F(StorageTests, LoadDocuments_WhenNestedContentExists_ReadsBackCorrectData) {
    Document nestedDoc;
    nestedDoc.set<std::string>("type", "inner");
    nestedDoc.set<int>("value", 99);

    Document::Vector vec = {
        []{
            Document d;
            d.set<std::string>("vtype", "v1");
            d.set<int>("vval", 11);
            return d;
        }(),
        []{
            Document d;
            d.set<std::string>("vtype", "v2");
            d.set<int>("vval", 22);
            return d;
        }()
    };

    Document::Map map = {
        { "m1", nestedDoc },
        { "m2", nestedDoc }
    };

    Document doc;
    doc.set<size_t>("id", 22);
    doc.set<std::string>("name", "Complex Doc");
    doc.set("nested", nestedDoc);
    doc.set("vector", vec);
    doc.set("map", map);

    storage.saveDocument(collectionPath, doc);

    auto loaded = storage.loadDocuments(collectionPath);
    ASSERT_EQ(loaded.size(), 1);

    const auto& loadedDoc = loaded[0];
    EXPECT_EQ(*loadedDoc.get<std::string>("name"), "Complex Doc");

    auto nested = *loadedDoc.get<Document>("nested");
    EXPECT_EQ(*nested.get<std::string>("type"), "inner");
    EXPECT_EQ(*nested.get<int>("value"), 99);

    auto loadedVec = *loadedDoc.get<Document::Vector>("vector");
    ASSERT_EQ(loadedVec.size(), 2);
    EXPECT_EQ(*loadedVec[0].get<std::string>("vtype"), "v1");
    EXPECT_EQ(*loadedVec[0].get<int>("vval"), 11);
    EXPECT_EQ(*loadedVec[1].get<std::string>("vtype"), "v2");
    EXPECT_EQ(*loadedVec[1].get<int>("vval"), 22);

    auto loadedMap = *loadedDoc.get<Document::Map>("map");
    ASSERT_EQ(loadedMap.size(), 2);
    EXPECT_EQ(*loadedMap["m1"].get<int>("value"), 99);
    EXPECT_EQ(*loadedMap["m2"].get<std::string>("type"), "inner");
}

// -------------------- Tests: removeDocument--------------------

TEST_F(StorageTests, RemoveDocument_DeletesFile) {
    auto doc = createSampleDocument(31);

    storage.saveDocument(collectionPath, doc);
    std::string path = collectionPath + "/31.txt";
    EXPECT_TRUE(std::filesystem::exists(path));

    storage.removeDocument(collectionPath, 31);
    EXPECT_FALSE(std::filesystem::exists(path));
}