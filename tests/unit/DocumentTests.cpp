#include "gtest/gtest.h"

#include "Document.hpp"

class DocumentTests : public ::testing::Test {
protected:
    Document doc;

    Document nested_doc;
    Document::Vector vec;
    Document::Map map;
    
    void SetUp() override {
        // simple types
        doc.set("int_val", 42);
        doc.set("size_t_val", static_cast<size_t>(123456789ULL));
        doc.set("double_val", 3.14159);
        doc.set("string_val", std::string("test_string"));
        doc.set("bool_val", true);
        
        // nested document
        nested_doc.set("nested_key", std::string("nested_value"));
        doc.set("document_val", nested_doc);

        // document vector
        vec.push_back(nested_doc);
        Document vecDoc;
        vecDoc.set("vec_item", 2);
        vec.push_back(vecDoc);
        doc.set("vector_val", vec);

        // document map
        map.emplace("map_key1", nested_doc);
        map.emplace("map_key2", nested_doc);
        Document mapDoc;
        mapDoc.set("map_item", false);
        doc.set("map_val", map);
    }
};

// -------------------- Tests: set --------------------

TEST_F(DocumentTests, Set_WhenKeyAlreadyExists_OverriteIt) {
    doc.set("test_key", 123);
    auto val = doc.get<int>("test_key");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 123);

    doc.set("test_key", 456);
    val = doc.get<int>("test_key");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 456);
}

TEST_F(DocumentTests, Get_WhenPassedKeyIsIdAndValueIsNotSizeT_ThrowException) {
    ASSERT_THROW(doc.set("id", 1), std::invalid_argument); 
}

// -------------------- Tests: get<T> --------------------

TEST_F(DocumentTests, Get_WhenIntExists_ReturnIt) {
    auto result = doc.get<int>("int_val");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST_F(DocumentTests, Get_WhenSizeTExists_ReturnIt) {
    auto result = doc.get<size_t>("size_t_val");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result , static_cast<size_t>(123456789ULL));
}

TEST_F(DocumentTests, Get_WhenDoubleExists_ReturnIt) {
    auto result = doc.get<double>("double_val");
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(*result, 3.14159);
}

TEST_F(DocumentTests, Get_WhenStringExist_ReturnIt) {
    auto result = doc.get<std::string>("string_val");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "test_string");
}

TEST_F(DocumentTests, Get_WhenBoolExists_ReturnIt) {
    auto result = doc.get<bool>("bool_val");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(*result);
}

TEST_F(DocumentTests, Get_WhenDocumentExists_ReturnIt) {
    auto result = doc.get<Document>("document_val");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->get<std::string>("nested_key"), "nested_value");
}

TEST_F(DocumentTests, Get_WhenVectorExists_ReturnIt) {
    auto result = doc.get<Document::Vector>("vector_val");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->size(), 2);
    EXPECT_EQ(result->at(0).get<std::string>("nested_key"), "nested_value");
}

TEST_F(DocumentTests, Get_WhenMapExists_ReturnIt) {
    auto result = doc.get<Document::Map>("map_val");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->size(), 2);
    EXPECT_FALSE(result->at("map_key2").get<bool>("map_item"));
}

TEST_F(DocumentTests, Get_WhenPassedMissingKey_ReturnNullopt) {
    auto result = doc.get<int>("nonexistent_key");
    EXPECT_FALSE(result.has_value());
}

TEST_F(DocumentTests, Get_WhenPassedWrongType_ReturnNullopt) {
    auto result = doc.get<std::string>("int_val");
    EXPECT_FALSE(result.has_value());
}

TEST(DocumentStandaloneTest, Get_WhenEmptyDoc_ReturnsNullopt) {
    Document d;
    auto result = d.get<int>("some_key");
    EXPECT_FALSE(result.has_value());
}

// -------------------- Tests: hasField --------------------

TEST_F(DocumentTests, HasField_WhenKeyExists_ReturnTrue) {
    EXPECT_TRUE(doc.hasField("int_val"));
    EXPECT_TRUE(doc.hasField("string_val"));
    EXPECT_TRUE(doc.hasField("document_val"));
    EXPECT_TRUE(doc.hasField("vector_val"));
    EXPECT_TRUE(doc.hasField("map_val"));
}

TEST_F(DocumentTests, HasField_WhenKeyDoesNotExist_ReturnFalse) {
    EXPECT_FALSE(doc.hasField("nonexistent_key"));
    EXPECT_FALSE(doc.hasField(""));
    EXPECT_FALSE(doc.hasField("random_key"));
}

TEST_F(DocumentTests, HasField_WhenNewKeyIsAdded_ReturnTrue) {
    EXPECT_FALSE(doc.hasField("new_field"));
    doc.set("new_field", 123);
    EXPECT_TRUE(doc.hasField("new_field"));
}

TEST_F(DocumentTests, HasField_WhenKeyIsRemoved_ReturnFalse) {
    doc.set("temp_field", 1);
    EXPECT_TRUE(doc.hasField("temp_field"));
    doc.remove("temp_field");
    EXPECT_FALSE(doc.hasField("temp_field"));
}

// -------------------- Tests: getDataView --------------------

TEST_F(DocumentTests, GetDataView_ReturnsAllFields) {
    const auto& view = doc.getDataView();
    EXPECT_GE(view.size(), 5);
    EXPECT_TRUE(view.find("int_val") != view.end());
}