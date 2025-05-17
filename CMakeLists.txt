cmake_minimum_required(VERSION 3.14)
project(TaskFramework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Zbiór plików nagłówkowych i źródłowych
include_directories(include)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    Main.cpp
)

add_executable(db ${SOURCES})

# Dodatkowe ostrzeżenia (dla GCC/Clang/MSVC)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(db PRIVATE -Wall -Wextra -pedantic -Werror)
elseif (MSVC)
    target_compile_options(db PRIVATE /W4 /WX)
endif()
