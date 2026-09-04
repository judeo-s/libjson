# Example executable.
ADD_EXECUTABLE(libjson_basic
    basic.cxx
)

# Link the library.
TARGET_LINK_LIBRARIES(libjson_basic
    PRIVATE
        libjson::json
)
