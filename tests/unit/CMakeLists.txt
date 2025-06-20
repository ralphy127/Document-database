add_executable(unit_tests
    DocumentTests.cpp
    CollectionTests.cpp
    StorageTests.cpp
    DatabaseTests.cpp
)

target_link_libraries(unit_tests PRIVATE
    GTest::GTest
    GTest::Main
    DatabaseCore 
)

gtest_discover_tests(unit_tests)

add_test(NAME unit_tests COMMAND unit_tests)