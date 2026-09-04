#pragma once

#include "libjson/json.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace libjson::detail {
class Parser {
public:
  explicit Parser(std::string_view input);
  Json parse();

private:
  std::string_view input_;
  std::size_t pos_{};

  Json parse_value();
  Json parse_null();
  Json parse_bool();
  Json parse_number();
  Json parse_array();
  Json parse_object();

  std::string parse_string();

  void skip_whitespace();
  void expect(char expected);
  void fail(const std::string &message) const;

  bool consume(char expected);
  bool match(std::string_view text);
  bool is_done() const;
  char peek() const;
  char get();

  static int hex_value(char c);
  static void append_utf8(std::string &output, unsigned int codepoint);
  unsigned int parse_unicode_escape();
};
} // namespace libjson::detail
