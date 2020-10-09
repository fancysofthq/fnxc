#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "../lua/cst.hpp"
#include "./cst.hpp"

namespace FNX {
namespace Onyx {

/// The Onyx Abstract Syntax Tree. It usually has a global,
/// program-wide instance, and is enriched by `Builder` instances.
class AST {
public:
private:
  /// Function CSTs with evaluated macros
  /// to be pointed by AST entries.
  std::vector<std::unique_ptr<Onyx::CST>> _function_csts;

  /// Note that these macro CSTs may be different from the source
  /// code CST\: they can emit nested contents if complex.
  std::vector<std::unique_ptr<Lua::CST>> _macro_csts;
};

} // namespace Onyx
} // namespace FNX
