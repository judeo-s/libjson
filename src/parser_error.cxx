#include "libjson/parser_error.hpp"
#include <strings.h>

namespace libjson {
parse_error::parse_error(const std::string &message, std::size_t index)
    : std::runtime_error(message + " at index " + std::to_string(index)),
      index_(index) {}

std::size_t parse_error::index() const noexcept { return index_; }
} // namespace libjson
