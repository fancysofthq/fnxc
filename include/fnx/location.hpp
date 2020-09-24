#pragma once

#include "./position.hpp"

namespace FNX {

/**
 * @brief A may-be-spanning location.
 */
struct Location {
  Position begin;
  Position end;

  /// @brief A spanning location constructor.
  Location(Position begin, Position end);

  /// @brief A non-spanning location constructor.
  Location(Position pos);
};

} // namespace FNX
