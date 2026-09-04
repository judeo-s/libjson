# Test dependency.
INCLUDE(FetchContent)

# Download GoogleTest.
FetchContent_Declare(googletest
    URL
        https://github.com/google/googletest/archive/refs/tags/v1.18.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# Test executable.
ADD_EXECUTABLE(libjson_unit_tests
    main.cxx
)

# Link test libraries.
TARGET_LINK_LIBRARIES(libjson_unit_tests
    PRIVATE
        libjson::json
        GTest::gtest
)

# Add test to CTest.
add_test(NAME libjson_unit_tests COMMAND libjson_unit_tests)
