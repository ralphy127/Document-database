cmake_minimum_required(VERSION 3.14)
project(TaskFramework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    Main.cpp
)

add_executable(db ${SOURCES})

target_compile_options(db PRIVATE -Wall -Wextra -pedantic -Werror)