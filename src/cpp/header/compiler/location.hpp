#pragma once
#include "./unit.hpp"

namespace Onyx {
namespace Compiler {
struct Position {
  unsigned long row;
  unsigned long col;

  Position(unsigned long row, unsigned long col) :
      row(row), col(col) {}

  Position() {}
};

// A location within a compilation unit (i.e. a file).
// It may be spanning, i.e. have different begin and end positions.
//
// ```
// foo
//   bar
// ```
//
// The "token" above would have spanning coordinates 0:0,1:5.
struct Location {
  const shared_ptr<Unit> unit;

  Position begin;
  Position end;

  Location(shared_ptr<Unit> unit, Position begin, Position end) :
      unit(unit), begin(begin), end(end) {}

  Location(shared_ptr<Unit> unit, Position pos) :
      unit(unit), begin(pos), end(begin) {}

  Location(shared_ptr<Unit> unit) :
      unit(unit), begin(Position(0, 0)), end(begin) {}
};
} // namespace Compiler
} // namespace Onyx
