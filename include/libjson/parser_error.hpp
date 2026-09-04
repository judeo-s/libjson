#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>

namespace libjson {

// Error thrown when JSON parsing fails.
class parse_error : public std::runtime_error {
private:
  std::size_t index_;

public:
  parse_error(const std::string &message, std::size_t index);
  std::size_t index() const noexcept;
};
} // namespace libjson
