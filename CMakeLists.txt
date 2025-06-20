cmake_minimum_required(VERSION 3.14)
project(TaskFramework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(DatabaseCore STATIC
    src/Collection.cpp
    src/Database.cpp
    src/Seeder.cpp
    src/Storage.cpp
)

target_include_directories(DatabaseCore PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(DatabaseCore PRIVATE 
    -Wall -Wextra -pedantic -Werror
)

add_executable(db Main.cpp)
target_link_libraries(db PRIVATE DatabaseCore)

enable_testing()
add_subdirectory(tests)