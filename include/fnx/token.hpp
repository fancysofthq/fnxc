#pragma once

#include "./location.hpp"
#include "./unit.hpp"

namespace FNX {

/// A generic source code token.
///
/// A token's responsibilty is to convey as much information as
/// possible from a source code piece to avoid excessive parsing.
///
/// Instead of copying source code contents, it points to some
/// location within the translation unit.
struct Token {
  /// Unit containing this token.
  std::shared_ptr<Unit> unit;

  /// Location of this token within the @link #unit @endlink.
  Location location;

  /// Index of this token in the @link #unit @endlink's
  /// @link Unit#tokens @endlink vector.
  uint32_t idx;
};

} // namespace FNX
