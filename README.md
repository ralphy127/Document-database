# Document-Based Database (C++)

A lightweight, template-based document database implemented in pure C++17, designed primarily as a learning project but flexible enough for various static data management use cases.

---

## Overview

This project demonstrates a document-oriented database system with support for collections and nested documents, all stored in a simple file-based format. It emphasizes static typing via templates and a flexible Value type that can hold various data kinds, enabling complex and hierarchical data structures to be represented efficiently and safely. The design aims to provide a foundational structure for managing and querying hierarchical data with type safety and extensibility.

---

## Features

- Document-based storage with support for nested documents  
- Simple file-backed collections  
- Template-driven static data structures  
- Basic seeding utility for example datasets  
- Unit tests using Google Test framework  

---

## Prerequisites

- A C++17 compatible compiler
- CMake  
- Google Test (for running unit tests)  

---

## Build & Run

Build the project using CMake from the `build` directory:

```bash
mkdir -p build
cd build
cmake ..
make
```

Run the main database program:
```bash
./db
```

Run all unit tests:

```bash
ctest
```

---

## Usage

The program loads data from the example_database folder, which contains collections with document files.

Modify or extend the Seeder utility (Seeder::seedDatabase in Seeder.cpp) to populate the database with custom data.

Explore the unit tests to better understand the functionality and usage patterns.

Documents support the following types (via the Value alias in Document class):
- int
- size_t
- double
- bool
- std::string
- std::vector<Document>
- std::unordered_map<std::string, Document>

Feel free to extend this set with your own custom types if needed.

---

## Basic Usage Example

```cpp
#include "Database.hpp"
#include "Seeder.hpp"

int main() {
    Database db("example_database");

    Collection col("my_collection");
    for (int i = 0; i < 3; ++i) {
        Document doc;
        doc.set("name", std::string("doc_") + std::to_string(i));
        doc.set("value", i);
        col.insert(doc);
    }
    db.insertCollection(std::move(col));

    auto results = db.find("my_collection", [](const Document& doc) {
        auto name = doc.get<std::string>("name");
        return name && *name == "doc_1";
    });

    std::cout << "Found " << results.size() << " document(s) with name 'doc_1'.\n";

    return 0;
}
```