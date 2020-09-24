#pragma once

#include <stdint.h>

namespace FNX {

/// A position comprising row and column in *code points*, and offset
/// in *bytes*. Note that the same row and column may have different
/// offsets, when a code point consists of multiple code units.
struct Position {
  /// Code point row number, beginning from zero.
  uint32_t row;

  /// Code point column number, beginning from zero.
  uint32_t col;

  /// Byte offset. A zero offset means
  /// that the source is not read yet.
  uint32_t offset;

  Position(uint32_t row, uint32_t col, uint32_t offset);
  Position();
};

} // namespace FNX
