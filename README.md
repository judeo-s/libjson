# libjson

A small C++20 JSON library for parsing and generating JSON.

## Usage

```cpp
#include "libjson/json.hpp"

#include <iostream>

int main() {
    libjson::Json json = libjson::Json::parse(R"({"name":"Alice","age":30})");

    std::cout << json["name"].as_string() << '\n';
    std::cout << json.stringify() << '\n';
}
```

## CMake FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    libjson
    GIT_REPOSITORY https://github.com/judeo-s/libjson.git
    GIT_TAG main
)

FetchContent_MakeAvailable(libjson)

target_link_libraries(my_app PRIVATE libjson::json)
```

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```
