#pragma once

#include <stack>
#include <stdexcept>

#include "./location.hpp"

namespace FNX {

/**
 * @brief A compiler panic due to syntax or semantic error.
 */
struct Panic : std::logic_error {
  /**
   * @brief Backtrace to the original panic location.
   *
   * It shall be pushed into while the panic is propagated.
   */
  std::stack<Location> backtrace;

  /**
   * @brief Construct a new Panic object.
   *
   * @param loc The original panic location.
   * @param msg The panic message.
   */
  Panic(Location loc, std::string msg);
};

} // namespace FNX
