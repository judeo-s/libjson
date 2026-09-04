#pragma once

#include <cstddef>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace libjson {

// Stores any JSON value: null, bool, number, string, array, or object.
class Json {
public:
  using array = std::vector<Json>;
  using object = std::map<std::string, Json>;

  Json();
  Json(std::nullptr_t);
  Json(bool value);
  Json(int value);
  Json(double value);
  Json(const char *value);
  Json(std::string value);
  Json(array value);
  Json(object value);

  bool is_null() const;
  bool is_bool() const;
  bool is_number() const;
  bool is_string() const;
  bool is_array() const;
  bool is_object() const;

  // Read the stored value as a specific type.
  bool as_bool() const;
  double as_number() const;
  const std::string &as_string() const;
  const array &as_array() const;
  const object &as_object() const;
  std::string &as_string();
  array &as_array();
  object &as_object();

  const Json &operator[](const std::string &key) const;
  Json &operator[](const std::string &key);

  const Json &operator[](const std::size_t index) const;
  Json &operator[](const std::size_t index);

  // Convert between text and Json values.
  static Json parse(std::string_view input);
  std::string stringify() const;

private:
  using storage =
      std::variant<std::nullptr_t, bool, double, std::string, array, object>;

  storage value_;

  void write_json(std::ostringstream &out) const;

  static void write_string(std::ostringstream &out, const std::string &value);
  static void write_array(std::ostringstream &out, const array &values);
  static void write_object(std::ostringstream &out, const object &values);
};

} // namespace libjson
