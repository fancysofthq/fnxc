#pragma once

#include "../unit.hpp"
#include <ostream>
#include <stdint.h>

namespace FNX {
namespace CNX {

/// An exported CNX block. At the moment of construction, it may have
/// unknown offsets.
class Block : FNX::Unit {
public:
  /// Construct a new Block object.
  ///
  /// @param input The input stream.
  Block(std::basic_ostream<char8_t> *input);

private:
  std::basic_ostream<char8_t> *_input;
};

} // namespace CNX
} // namespace FNX
