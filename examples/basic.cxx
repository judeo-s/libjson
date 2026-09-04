#include "libjson/json.hpp"
#include "libjson/parser_error.hpp"
#include <iostream>

using namespace std::string_view_literals;
int main() {
  try {
    libjson::Json json = libjson::Json::parse(R"({
    "name" : "Alice",
    "age" : 30,
    "admin" : false,
    "skills" : ["cpp", "json"],
    "address" : null
    })");

    std::cout << json["name"].as_string() << '\n';
    std::cout << json["skills"][0].as_string() << '\n';

    json["active"] = true;
    std::cout << json.stringify() << '\n';
  } catch (const libjson::parse_error &error) {
    std::cerr << "Parse error: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
