cmake_minimum_required(VERSION 3.14)

# Project info.
project(libjson VERSION 1.0.0 LANGUAGES CXX)

# Check if this is the main project.
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(LIBJSON_IS_TOP_LEVEL ON)
else()
    set(LIBJSON_IS_TOP_LEVEL OFF)
endif()

# Only build tests and examples by default in this repo.
option(LIBJSON_BUILD_TESTS "Build libjson tests" ${LIBJSON_IS_TOP_LEVEL})
option(LIBJSON_BUILD_EXAMPLES "Build libjson examples" ${LIBJSON_IS_TOP_LEVEL})

# Main library.
add_library(libjson
    src/json.cxx
    src/parser.cxx
    src/parser_error.cxx
)

# Public target name for users.
add_library(libjson::json ALIAS libjson)

# Output libjson.a instead of liblibjson.a.
set_target_properties(libjson PROPERTIES
    OUTPUT_NAME json
)

# This library needs C++20.
target_compile_features(libjson PUBLIC cxx_std_20)

# Public include folder.
target_include_directories(libjson
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Unit tests.
if(LIBJSON_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Example programs.
if(LIBJSON_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
