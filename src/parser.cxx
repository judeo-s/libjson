#include "libjson/parser.hpp"
#include "libjson/json.hpp"
#include "libjson/parser_error.hpp"

#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <string_view>
#include <utility>

namespace libjson::detail {
Parser::Parser(std::string_view input) : input_(input) {}

// Parse one complete JSON document.
Json Parser::parse() {
  Json result = parse_value();
  skip_whitespace();
  if (!is_done())
    fail("unexpected trailing characters");
  return result;
}

// Choose the parser function based on the next character.
Json Parser::parse_value() {
  skip_whitespace();
  if (is_done())
    fail("unexpected end of input");
  char c = peek();

  if (c == 'n')
    return parse_null();
  if (c == 't' || c == 'f')
    return parse_bool();
  if (c == '"')
    return Json(parse_string());
  if (c == '[')
    return parse_array();
  if (c == '{')
    return parse_object();
  if (c == '-' || std::isdigit(static_cast<unsigned char>(c)))
    return parse_number();

  fail("unexpected characters");
  return parse_null();
}

Json Parser::parse_null() {
  if (!match("null"))
    fail("invalid null value");
  return Json(nullptr);
}

Json Parser::parse_bool() {
  if (match("true"))
    return Json(true);
  if (match("false"))
    return Json(false);
  fail("invalid boolean value");
  return parse_null();
}

// Parse a JSON number and store it as a double.
Json Parser::parse_number() {
  std::size_t start = pos_;

  consume('-');

  if (is_done())
    fail("invalid number");

  if (consume('0')) {
    if (!is_done() && std::isdigit(static_cast<unsigned char>(peek())))
      fail("leading zero in number");
  } else {
    if (!std::isdigit(static_cast<unsigned char>(peek())))
      fail("invalid number");
    while (!is_done() && std::isdigit(static_cast<unsigned char>(peek())))
      ++pos_;
  }

  if (consume('.')) {
    if (is_done() || !std::isdigit(static_cast<unsigned char>(peek())))
      fail("expected digit after decimal point");
    while (!is_done() && std::isdigit(static_cast<unsigned char>(peek())))
      ++pos_;
  }

  if (!is_done() && (peek() == 'e' || peek() == 'E')) {
    ++pos_;
    if (!is_done() && (peek() == '+' || peek() == '-'))
      ++pos_;

    if (is_done() || !std::isdigit(static_cast<unsigned char>(peek())))
      fail("expected digit in exponent");

    while (!is_done() && std::isdigit(static_cast<unsigned char>(peek())))
      ++pos_;
  }
  std::string number(input_.substr(start, pos_ - start));
  char *end = nullptr;
  double value = std::strtod(number.c_str(), &end);

  if (end != number.c_str() + number.size())
    throw parse_error("invalid number", start);

  return Json(value);
}

// Parse a JSON array: [value, value, ...].
Json Parser::parse_array() {
  expect('[');
  Json::array result;
  skip_whitespace();
  if (consume(']'))
    return Json(std::move(result));

  while (true) {
    result.push_back(parse_value());
    skip_whitespace();
    if (consume(']'))
      break;
    expect(',');
  }
  return Json(std::move(result));
}

// Parse a JSON object: {"key": value, ...}.
Json Parser::parse_object() {
  expect('{');
  Json::object result;
  skip_whitespace();
  if (consume('}'))
    return Json(std::move(result));

  while (true) {
    skip_whitespace();
    if (is_done() || peek() != '"')
      fail("expected object key");
    std::string key = parse_string();

    skip_whitespace();
    expect(':');
    Json value = parse_value();
    result[std::move(key)] = std::move(value);

    skip_whitespace();
    if (consume('}'))
      break;
    expect(',');
  }
  return Json(std::move(result));
}

// Parse a JSON string and handle escape sequences.
std::string Parser::parse_string() {
  expect('"');
  std::string result;

  while (!is_done()) {
    char c = get();
    if (c == '"')
      return result;
    if (static_cast<unsigned char>(c) < 0x20)
      fail("control character in string");
    if (c != '\\') {
      result.push_back(c);
      continue;
    }
    if (is_done())
      fail("unfinished escape sequence");

    char escaped = get();
    switch (escaped) {
    case '"':
      result.push_back('"');
      break;
    case '\\':
      result.push_back('\\');
      break;
    case '/':
      result.push_back('/');
      break;
    case 'b':
      result.push_back('\b');
      break;
    case 'f':
      result.push_back('\f');
      break;
    case 'n':
      result.push_back('\n');
      break;
    case 'r':
      result.push_back('\r');
      break;
    case 't':
      result.push_back('\t');
      break;
    case 'u': {
      unsigned int codepoint = parse_unicode_escape();
      if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
        if (!match("\\u"))
          fail("expected low surrogate");

        unsigned int low = parse_unicode_escape();
        if (low < 0xDC00 || low > 0xDFFF)
          fail("invalid low surrogate");

        codepoint = 0x10000 + (((codepoint - 0xD800) << 10) + (low - 0xDC00));
      } else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF)
        fail("unexpected low surrogate");

      append_utf8(result, codepoint);
      break;
    }
    default:
      fail("invalid escape sequence");
    }
  }
  fail("unterminated string");
  return "";
}

// Skip spaces allowed between JSON tokens.
void Parser::skip_whitespace() {
  while (!is_done()) {
    char c = peek();
    if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
      break;
    ++pos_;
  }
}

void Parser::expect(char expected) {
  using namespace std::string_literals;
  if (!consume(expected))
    fail(std::string("expected '"s + expected + "'"s));
}

void Parser::fail(const std::string &message) const {
  throw parse_error(message, pos_);
}

bool Parser::consume(char expected) {
  if (!is_done() && peek() == expected) {
    ++pos_;
    return true;
  }
  return false;
}

bool Parser::match(std::string_view text) {
  if (input_.substr(pos_, text.size()) == text) {
    pos_ += text.size();
    return true;
  }
  return false;
}

bool Parser::is_done() const { return pos_ >= input_.size(); }

char Parser::peek() const { return input_[pos_]; }

char Parser::get() { return input_[pos_++]; }

int Parser::hex_value(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return 10 + c - 'a';
  if (c >= 'A' && c <= 'F')
    return 10 + c - 'A';
  return -1;
}

// Parse four hex digits after \u.
unsigned int Parser::parse_unicode_escape() {
  unsigned int codepoint = 0;
  for (int i = 0; i < 4; ++i) {
    if (is_done())
      fail("invalid unicode escape");

    int value = hex_value(get());
    if (value < 0)
      fail("invalid unicode escape");

    codepoint = (codepoint << 4) | static_cast<unsigned int>(value);
  }
  return codepoint;
}

// Store a unicode codepoint as UTF-8.
void Parser::append_utf8(std::string &output, unsigned int codepoint) {
  if (codepoint <= 0x7F)
    output.push_back(static_cast<char>(codepoint));
  else if (codepoint <= 0x7FF) {
    output.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
    output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
  } else if (codepoint <= 0xFFFF) {
    output.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
    output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
    output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
  } else if (codepoint <= 0x10FFFF) {
    output.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
    output.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
    output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
    output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
  } else
    throw parse_error("invalid unicode codepoint", 0);
}
} // namespace libjson::detail
