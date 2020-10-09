#pragma once

#include <iostream>
#include <memory>
#include <vector>

namespace FNX {

// Forward declaration is needed here.
struct Token;

/// A translation unit.
/// It may be a physical file or a virtual code block.
class Unit {
public:
  /// The stream of source code for this unit.
  virtual std::basic_iostream<char8_t> *source_stream();
};

} // namespace FNX
