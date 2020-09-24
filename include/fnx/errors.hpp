#pragma once

#include <exception>
#include <string>

namespace FNX {

/**
 * @brief Internal Compiler Error, i.e. a compiler bug.
 */
struct ICE : std::exception {
  std::string message;
  ICE(std::string message = NULL) : message(message) {}
};

} // namespace FNX
