#pragma once

#include <filesystem>
#include <set>
#include <sstream>
#include <stdint.h>
#include <vector>

#include "../position.hpp"
#include "../unit.hpp"

namespace FNX {
namespace Onyx {
// The forward declaration is needed to
// avoid circular dependency issues.
//

struct Token;

namespace AST {
struct Node;
} // namespace AST

/// A physical file containing Onyx source code.
/// Its `source_stream` is read from the `bytes` buffer.
class File : public Unit {
public:
  /// Local AST root node for the file.
  const std::shared_ptr<AST::Node> ast;

  /// Path to the source file.
  const std::filesystem::path file_path;

  /// Read bytes of the file.
  const std::stringstream bytes;

  /// Return a pointer to the `bytes` memory buffer.
  std::iostream *source_stream() override;

  /// Set of files depending on this file.
  /// @note Circular dependencies are not possible.
  const std::set<std::weak_ptr<File>> dependants;
};
} // namespace Onyx
} // namespace FNX
