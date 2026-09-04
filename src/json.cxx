#include "libjson/json.hpp"
#include "libjson/parser.hpp"
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>
#include <variant>

namespace libjson {
Json::Json() : value_{nullptr} {}

Json::Json(std::nullptr_t) : value_{nullptr} {}

Json::Json(bool value) : value_{value} {}

Json::Json(int value) : value_{static_cast<double>(value)} {}

Json::Json(double value) : value_{value} {}

Json::Json(const char *value) : value_{std::string(value)} {}

Json::Json(std::string value) : value_{std::move(value)} {}

Json::Json(array value) : value_{std::move(value)} {}

Json::Json(object value) : value_{std::move(value)} {}

bool Json::is_null() const {
  return std::holds_alternative<std::nullptr_t>(value_);
}

bool Json::is_bool() const { return std::holds_alternative<bool>(value_); }

bool Json::is_number() const { return std::holds_alternative<double>(value_); }

bool Json::is_string() const {
  return std::holds_alternative<std::string>(value_);
}

bool Json::is_array() const { return std::holds_alternative<array>(value_); }

bool Json::is_object() const { return std::holds_alternative<object>(value_); }

bool Json::as_bool() const { return std::get<bool>(value_); }

double Json::as_number() const { return std::get<double>(value_); }

const std::string &Json::as_string() const {
  return std::get<std::string>(value_);
}

const Json::array &Json::as_array() const { return std::get<array>(value_); }

const Json::object &Json::as_object() const { return std::get<object>(value_); }

std::string &Json::as_string() { return std::get<std::string>(value_); }

Json::array &Json::as_array() { return std::get<array>(value_); }

Json::object &Json::as_object() { return std::get<object>(value_); }

// Object lookup by key.
const Json &Json::operator[](const std::string &key) const {
  return std::get<object>(value_).at(key);
}
Json &Json::operator[](const std::string &key) {
  return std::get<object>(value_)[key];
}

// Array lookup by index.
const Json &Json::operator[](std::size_t index) const {
  return std::get<array>(value_).at(index);
}
Json &Json::operator[](std::size_t index) {
  return std::get<array>(value_).at(index);
}

// Parse JSON text into a Json value.
Json Json::parse(std::string_view input) {
  detail::Parser parser(input);
  return parser.parse();
}

// Convert a Json value back to JSON text.
std::string Json::stringify() const {
  std::ostringstream out;
  write_json(out);
  return out.str();
}

// Write the current value based on its type.
void Json::write_json(std::ostringstream &out) const {
  if (is_null())
    out << "null";
  else if (is_bool())
    out << (as_bool() ? "true" : "false");
  else if (is_number())
    out << std::setprecision(15) << as_number();
  else if (is_string())
    write_string(out, as_string());
  else if (is_array())
    write_array(out, as_array());
  else if (is_object())
    write_object(out, as_object());
}

// Escape characters that need special handling in JSON strings.
void Json::write_string(std::ostringstream &out, const std::string &value) {
  out << '"';
  for (char c : value) {
    switch (c) {
    case '"':
      out << "\\\"";
      break;
    case '\\':
      out << "\\\\";
      break;
    case '\b':
      out << "\\b";
      break;
    case '\f':
      out << "\\f";
      break;
    case '\n':
      out << "\\n";
      break;
    case '\r':
      out << "\\r";
      break;
    case '\t':
      out << "\\t";
      break;
    default:
      if (static_cast<unsigned char>(c) < 0x20) {
        out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
            << static_cast<int>(static_cast<unsigned char>(c)) << std::dec
            << std::setfill(' ');
      } else {
        out << c;
      }
      break;
    }
  }
  out << '"';
}

void Json::write_array(std::ostringstream &out, const array &values) {
  out << '[';
  for (std::size_t i = 0; i < values.size(); ++i) {
    if (i > 0)
      out << ',';
    values[i].write_json(out);
  }
  out << ']';
}

void Json::write_object(std::ostringstream &out, const object &values) {
  out << '{';
  bool first = true;
  for (const auto &[key, value] : values) {
    if (!first)
      out << ',';
    first = false;

    write_string(out, key);
    out << ':';
    value.write_json(out);
  }
  out << '}';
}
} // namespace libjson
