#pragma once

#include <iostream>
#include <memory>
#include <vector>

namespace FNX {

// Forward declaration is needed here.
struct Token;

/**
 * @brief A translation unit.
 *
 * It may be a physical file or a virtual code block.
 */
class Unit {
public:
  /// The stream of source code for this unit.
  virtual std::iostream *source_stream();

  /// Tokens of this unit.
  std::vector<std::shared_ptr<Token>> tokens;
};

} // namespace FNX
